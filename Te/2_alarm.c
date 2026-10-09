/*
 * 2_alarm.c — 정해진 시간 뒤에 시그널을 받는다
 *
 * [핵심 개념]
 *   alarm(n) 은 n 초 뒤에 커널이 SIGALRM 을 보내도록 예약한다.
 *   시간 제한(타임아웃)을 구현하는 가장 간단한 방법이다.
 *   출처: man 2 alarm, man 2 sigaction
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 2_alarm 2_alarm.c
 *   ./2_alarm      # 3초 안에 뭔가 입력하지 않으면 시간이 끝난다
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static volatile sig_atomic_t alarm_fired = 0;

static void on_alarm(int sig)
{
    (void)sig;
    alarm_fired = 1;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("사용법: %s <간격초> <반복횟수>\n", argv[0]);
        printf("예시  : %s 2 5\n", argv[0]);
        return 1;
    }

    int interval = atoi(argv[1]);
    int max_repeat = atoi(argv[2]);

    if (interval <= 0 || max_repeat <= 0) {
        printf("간격초와 반복횟수는 1 이상의 양의 정수여야 합니다.\n");
        return 1;
    }

    struct sigaction sa;
    sa.sa_handler = on_alarm;   /* SIGALRM 이 오면 이 함수를 부르게 등록한다 */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;   /* SA_RESTART 를 안 줬으므로, 이 시그널은 fgets 같은 블로킹 호출을 중단시킨다 */
    sigaction(SIGALRM, &sa, NULL);

    printf("%d초 간격으로 총 %d번 타이머를 실행합니다.\n\n", interval, max_repeat);

    int count = 0;
    alarm(interval);

    while (count < max_repeat) {
        pause(); /* SIGALRM 시그널 수신 대기 */

        if (alarm_fired) {
            alarm_fired = 0;
            count++;
            printf("[%d/%d] %d초 타이머 알람 발생!\n", count, max_repeat, interval);

            /* 남은 횟수가 있다면 다시 alarm 예약 */
            if (count < max_repeat) {
                alarm(interval);
            }
        }
    }
    
    printf("\n타이머 %d회 반복 완수. 프로그램을 종료합니다.\n", max_repeat);
    return 0;
}
