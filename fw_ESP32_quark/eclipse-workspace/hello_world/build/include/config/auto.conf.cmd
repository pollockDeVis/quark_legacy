deps_config := \
	/home/Pollock/esp/esp-idf/components/app_trace/Kconfig \
	/home/Pollock/esp/esp-idf/components/aws_iot/Kconfig \
	/home/Pollock/esp/esp-idf/components/bt/Kconfig \
	/home/Pollock/esp/esp-idf/components/driver/Kconfig \
	/home/Pollock/esp/esp-idf/components/esp32/Kconfig \
	/home/Pollock/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/home/Pollock/esp/esp-idf/components/esp_event/Kconfig \
	/home/Pollock/esp/esp-idf/components/esp_http_client/Kconfig \
	/home/Pollock/esp/esp-idf/components/esp_http_server/Kconfig \
	/home/Pollock/esp/esp-idf/components/ethernet/Kconfig \
	/home/Pollock/esp/esp-idf/components/fatfs/Kconfig \
	/home/Pollock/esp/esp-idf/components/freemodbus/Kconfig \
	/home/Pollock/esp/esp-idf/components/freertos/Kconfig \
	/home/Pollock/esp/esp-idf/components/heap/Kconfig \
	/home/Pollock/esp/esp-idf/components/libsodium/Kconfig \
	/home/Pollock/esp/esp-idf/components/log/Kconfig \
	/home/Pollock/esp/esp-idf/components/lwip/Kconfig \
	/home/Pollock/esp/esp-idf/components/mbedtls/Kconfig \
	/home/Pollock/esp/esp-idf/components/mdns/Kconfig \
	/home/Pollock/esp/esp-idf/components/mqtt/Kconfig \
	/home/Pollock/esp/esp-idf/components/nvs_flash/Kconfig \
	/home/Pollock/esp/esp-idf/components/openssl/Kconfig \
	/home/Pollock/esp/esp-idf/components/pthread/Kconfig \
	/home/Pollock/esp/esp-idf/components/spi_flash/Kconfig \
	/home/Pollock/esp/esp-idf/components/spiffs/Kconfig \
	/home/Pollock/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/home/Pollock/esp/esp-idf/components/vfs/Kconfig \
	/home/Pollock/esp/esp-idf/components/wear_levelling/Kconfig \
	/home/Pollock/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/Pollock/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/Pollock/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/Pollock/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
