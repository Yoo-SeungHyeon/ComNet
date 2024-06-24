#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

// 함수 선언 수정
void get_name(char* input, int size, char* output) {
    // 결과 배열을 동적 할당
    strncpy(output, input + 1, size - 2); // 양 끝 대괄호 제외
    output[size - 2] = '\0'; // null-terminated string으로 만들기
}

int main() {
    char name[20] = "[ysh]";
    char result[20]; // 결과를 저장할 배열

    // 함수 호출 시 올바른 매개변수 전달
    get_name(name, strlen(name), result);
    printf("%s\n", result);

    return 0;
}
