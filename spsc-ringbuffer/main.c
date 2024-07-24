#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>

#ifndef SHM_HUGETLB
#define SHM_HUGETLB 04000
#endif



#define ADDR (void *)(0x0UL)
#define SHMAT_FLAGS (0)


#define HUGEPAGE_SIZE (256UL * 1024 * 1024)  // 256MB hugepages
// #define HUGEPAGE_SIZE (512UL * 1024 * 1024)  // 256MB hugepages
#define BUFFER_SIZE  (HUGEPAGE_SIZE)     // One hugepage size for the buffer
// https://abhinavag.medium.com/a-fast-circular-ring-buffer-4d102ef4d4a3

// https://github.com/google/wuffs/blob/main/script/mmap-ring-buffer.c
// https://github.com/lava/linear_ringbuffer/blob/master/include/bev/linear_ringbuffer.hpp
// https://github.com/smcho-kr/magic-ring-buffer
// https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/
// https://mikeash.com/pyblog/friday-qa-2012-02-03-ring-buffers-and-mirrored-memory-part-i.html
// https://fgiesen.wordpress.com/2012/07/21/the-magic-ring-buffer/
// 

int main() {
    // int shmid;
	unsigned long i;
	// char *shmaddr;

	// if ((shmid = shmget(2, HUGEPAGE_SIZE,
	// 		    SHM_HUGETLB | IPC_CREAT | SHM_R | SHM_W)) < 0) {
	// 	perror("shmget");
	// 	exit(1);
	// }
	// printf("shmid: 0x%x\n", shmid);

	// shmaddr = shmat(shmid, ADDR, SHMAT_FLAGS);
	// if (shmaddr == (char *)-1) {
	// 	perror("Shared memory attach failure");
	// 	shmctl(shmid, IPC_RMID, NULL);
	// 	exit(2);
	// }
	// printf("shmaddr: %p\n", shmaddr);

    // Allocate hugepages using mmap
    char *buf = mmap(ADDR, HUGEPAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
    // Do a truncate to HUGEPAGE_SIZE
    if (buf == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }



    for(int j=0; j<100; j++) {
        printf("Starting the writes:\n");
        for (i = 0; i < HUGEPAGE_SIZE; i++) {
            buf[i] = (char)(i);
            if (!(i % (1024 * 1024)))
                printf(".");
        }
        printf("\n");

        printf("Starting the Check...");
        for (i = 0; i < HUGEPAGE_SIZE; i++)
            if (buf[i] != (char)i)
                printf("\nIndex %lu mismatched\n", i);
        printf("Done.\n");
    }

    munmap(buf, HUGEPAGE_SIZE);
    return 0;
}
