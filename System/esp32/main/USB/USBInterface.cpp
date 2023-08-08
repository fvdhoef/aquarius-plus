#include "USBInterface.h"

static const char *TAG = "USBInterface";

USBInterface::USBInterface(USBDevice *_device)
    : device(_device) {
}

USBInterface::~USBInterface() {
}

void USBInterface::_interruptInTransferCb(usb_transfer_t *transfer) {
    if (transfer->status == USB_TRANSFER_STATUS_NO_DEVICE) {
        // Device is removed, free transfer
        ESP_LOGI(TAG, "inTransferCb - no device");
        usb_host_transfer_free(transfer);
        return;
    } else if (transfer->status == USB_TRANSFER_STATUS_COMPLETED) {
        static_cast<USBInterface *>(transfer->context)->processInterruptData(transfer->data_buffer, transfer->actual_num_bytes);
    }

    // Retransmit transfer to get next data
    usb_host_transfer_submit(transfer);
}
