#include "sdkconfig.h"
//#define _GLIBCXX_USE_C99

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <stdio.h>
#include <string>
#include "math.h"
#include "string.h"
#include "Esp32Gong.h"
#include "wavdata.h"
#include "I2SPlayer.hpp"
#include "SpiffsFileSystem.hpp"
#include "WebServer.hpp"
#include "Wifi.hpp"
#include "Config.hpp"

#define ONBOARDLED_GPIO GPIO_NUM_5  // GPIO5 on Sparkfun ESP32 Thing
#define LOGTAG "main"

I2SPlayer musicPlayer;
WebServer webServer;
Esp32Gong esp32gong;
SpiffsFileSystem spiffsFileSystem;
Wifi wifi;
Config config;

// Wav* wav = NULL;
Esp32Gong::Esp32Gong() {
//	wav = NULL;
}

Esp32Gong::~Esp32Gong() {
//	delete wav;
}

extern "C" {
void app_main();
}

void app_main() {
	nvs_flash_init();
	tcpip_adapter_init();
	esp32gong.Start();
}

//===========================================

void task_function_webserver(void *pvParameter) {
	((Esp32Gong*) pvParameter)->TaskWebServer();
	vTaskDelete(NULL);
}

void task_function_display(void *pvParameter) {
	((Esp32Gong*) pvParameter)->TaskDisplay();
	vTaskDelete(NULL);
}

//----------------------------------------------------------------------------------------

void Esp32Gong::Start() {

	ESP_LOGI(LOGTAG, "Welcome to Bernd's ESP32 Gong");

	/*musicPlayer.init();
	 musicPlayer.prepareWav(wavdata_h, sizeof(wavdata_h));
	 musicPlayer.playAsync();*/

	mbButtonPressed = !gpio_get_level(GPIO_NUM_0);
	config.Read();

	gpio_pad_select_gpio(10);
	gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
	gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);

	gpio_pad_select_gpio((gpio_num_t) ONBOARDLED_GPIO);
	gpio_set_direction((gpio_num_t) ONBOARDLED_GPIO, (gpio_mode_t) GPIO_MODE_OUTPUT);

	xTaskCreate(&task_function_webserver, "Task_WebServer", 4096, this, 5, NULL);
	xTaskCreate(&task_function_display, "Task_LED", 4096, this, 5, NULL);

	if (config.mbAPMode) {
		if (config.muLastSTAIpAddress) {
			char sBuf[16];
			sprintf(sBuf, "%d.%d.%d.%d", IP2STR((ip4_addr* )&config.muLastSTAIpAddress));
			ESP_LOGD(LOGTAG, "Last IP when connected to AP: %d : %s", config.muLastSTAIpAddress, sBuf);
		}
		wifi.StartAPMode(config.msAPSsid, config.msAPPass);
	} else {
		if (config.msSTAENTUser.length())
			wifi.StartSTAModeEnterprise(config.msSTASsid, config.msSTAENTUser, config.msSTAPass, config.msSTAENTCA);
		else
			wifi.StartSTAMode(config.msSTASsid, config.msSTAPass);
	}

	//TODO
	//wifi.StartMDNS();

}

void Esp32Gong::TaskWebServer() {
	webServer.start();
}

void Esp32Gong::TaskDisplay() {
	int level = 0;
	int ticks = 0;

	while (1) {
		if (wifi.IsConnected() && mbApiCallReceived) {
			gpio_set_level((gpio_num_t) ONBOARDLED_GPIO, (gpio_mode_t) level);
			level = !level;
			ticks = 0;
		} else {
			if (ticks > 1) { // blink half speed
				level = !level;
				ticks = 0;
			}
		}
		ticks++;

		gpio_set_level((gpio_num_t) ONBOARDLED_GPIO, (gpio_mode_t) level);
		vTaskDelay(500 / portTICK_PERIOD_MS);

		if (!gpio_get_level(GPIO_NUM_0)) {
			if (!mbButtonPressed) {
				ESP_LOGI(LOGTAG, "Factory settings button pressed... rebooting into Access Point mode.");
				config.ToggleAPMode();
				config.Write();
				esp_restart();
			}
		} else {
			mbButtonPressed = false;
		}

		vTaskDelay(1);
	}
}

//-----------------------------------------------------------------------------------------

