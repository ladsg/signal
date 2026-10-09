/*
 * 3_signal_block.c — 시그널 차단(Block) 및 차단 해제 시Pending 전달 확인
 *
 * [핵심 개념]
 *   sigprocmask 로 시그널을 차단하면 시그널이 소멸하지 않고 대기(pending) 상태로 쌓인다.
 *   차단을 해제하는 순간 커널에 대기 중이던 시그널이 핸들러로 전달된다.
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 3_signal_block 3_signal_block.c
 *   ./3_signal_block
 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static volatile sig_atomic_t got_sigint = 0;

static void handler(int sig)
{
    (void)sig;
    got_sigint = 1; /* 차단이 풀려서 시그널이 들어오면 플래그를 1로 변경 */
}

int main(void)
{
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    sigset_t block, old;
    sigemptyset(&block);
    sigaddset(&block, SIGINT); /* SIGINT 차단 집합 생성 */

    printf("=== [1단계] SIGINT 차단 시작 ===\n");
    if (sigprocmask(SIG_BLOCK, &block, &old) == -1) {
        perror("sigprocmask");
        return 1;
    }

    printf("지금부터 5초간 Ctrl+C(SIGINT)가 차단됩니다. 지금 Ctrl+C를 여러 번 눌러보세요!\n");
    for (int i = 5; i > 0; i--) {
        printf("%d초 남음...\n", i);
        sleep(1);
    }

    printf("\n=== [2단계] 차단 구간 종료 ===\n");
    printf("차단된 5초 동안 핸들러 실행 여부: %s\n", got_sigint ? "실행됨 (에러)" : "실행 안 됨 (차단 성공)");

    printf("\n=== [3단계] 차단 해제 (sigprocmask SIG_SETMASK) ===\n");
    printf("차단을 해제합니다. 대기 중이던 시그널이 전달됩니다...\n");
    fflush(stdout);

    /* 마스크를 원복하는 바로 이 순간! 대기(Pending) 중이던 SIGINT가 즉시 핸들러로 전달됨 */
    sigprocmask(SIG_SETMASK, &old, NULL);

    /* 차단 해제 직후 즉시 핸들러가 수신했는지 확인 */
    if (got_sigint) {
        printf("\n[성공] 차단 해제 직후 대기 중이던 SIGINT 시그널을 성공적으로 전달받아 처리했습니다!\n");
    } else {
        printf("\n[참고] 차단된 동안 Ctrl+C를 누르지 않아 대기 중인 시그널이 없었습니다.\n");
    }

    printf("프로그램을 종료합니다.\n");
    return 0;
}