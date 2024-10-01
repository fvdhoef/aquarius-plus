#include "Common.h"
#include <atomic>

class IdxAlloc {
public:
    uint8_t val;

    int alloc() {
        vTaskSuspendAll();

        int result = -1;
        for (int i = 0; i < 8; i++) {
            if ((val & (1 << i)) == 0) {
                result = i;
                val |= (1 << i);
                break;
            }
        }

        xTaskResumeAll();

        return result;
    }

    void free(int idx) {
        if (idx < 0 || idx > 7)
            return;

        vTaskSuspendAll();
        val |= (1 << idx);
        xTaskResumeAll();
    }
};
