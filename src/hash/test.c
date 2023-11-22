#include <stdio.h>
#include <unistd.h>

int main() {
    int n = 1;
    while (1) {
        printf("%d\n", n);
        n = n + 1;
        sleep(1); // 1초 딜레이
    }

    return 0;
}
