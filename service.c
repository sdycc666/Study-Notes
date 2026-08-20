// service.c
// HTTP服务器 - 请求处理端

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char const *argv[])
{
        int s_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (s_fd == -1) {
        perror("socket创建失败");
        exit(EXIT_FAILURE);
    }


    int opt = 1;
    if (setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt失败");
        close(s_fd);
        exit(EXIT_FAILURE);
    }


    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    if (bind(s_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind失败");
        close(s_fd);
        exit(EXIT_FAILURE);
    }


    if (listen(s_fd, 10) == -1) {
        perror("listen失败");
        close(s_fd);
        exit(EXIT_FAILURE);
    }

    printf("✓ HTTP服务器已启动，监听端口: 8080\n");
    printf("  访问地址: http://localhost:8080\n\n");


    const char *html_content = 
        "<html><body>"
        "<h1>Hello! 浏览器连接成功！</h1>"
        "<p>这是一个简单的HTTP服务器响应</p>"
        "</body></html>";
    
    char response[4096];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        strlen(html_content), html_content);


    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        
        int client_fd = accept(s_fd, (struct sockaddr*)&client_addr, &len);
        if (client_fd == -1) {
            perror("accept失败");
            continue;
        }

        printf("✓ 新连接: %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port));

        // 读取请求（只打印，不解析）
        char buffer[4096] = {0};
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            printf("收到请求:\n%s\n", buffer);
        }

        // 发送响应
        write(client_fd, response, strlen(response));

        // 关闭连接
        close(client_fd);
        printf("✓ 响应已发送，连接已关闭\n\n");
    }

    close(s_fd);
    return 0;
}