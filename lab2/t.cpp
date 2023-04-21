#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main() {
    // 定義共享記憶體的 key
    key_t key = ftok("shared_memory_example", 65);

    // 建立共享記憶體區域，大小為一個整數
    int shm_id = shmget(key, sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        return 1;
    }

    // 將共享記憶體映射到進程的記憶體空間
    int* shm_ptr = (int*)shmat(shm_id, NULL, 0);
    if (shm_ptr == (int*)-1) {
        perror("shmat");
        return 1;
    }

    // 寫入整數值到共享記憶體
    *shm_ptr = 42;
    std::cout << "Write value to shared memory: " << *shm_ptr << std::endl;

    // 分離共享記憶體映射
    if (shmdt(shm_ptr) == -1) {
        perror("shmdt");
        return 1;
    }

    // 在另一個進程中讀取共享記憶體的值
    int* shm_ptr_read = (int*)shmat(shm_id, NULL, 0);
    if (shm_ptr_read == (int*)-1) {
        perror("shmat");
        return 1;
    }

    // 讀取共享記憶體的值並輸出
    std::cout << "Read value from shared memory: " << *shm_ptr_read << std::endl;

    // 分離共享記憶體映射
    if (shmdt(shm_ptr_read) == -1) {
        perror("shmdt");
        return 1;
    }

    // 刪除共享記憶體區域
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        return 1;
    }

    return 0;
}
