/* UART asynchronous example, that uses separate RX and TX tasks

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include <string.h>

int step = 1, stepError = 0,flag = 1;
static const int RX_BUF_SIZE = 1024;
//char RSRP[4], RSRQ[4], SINR[3], PCI[10], cellID[20], latitude[10], longitude[11], date[18], temp[200];
int rsrp, rsrq, sinr, pci, cellID, others, dataSendSize;
float latitude, longitude, date;
char temp[200], dataSend[200]; //4 + 4 + 3 + 10 + 20 + 10 + 11 + 18 = 80, ngoai ra "":*8 = 24, ten: 4 + 4 + 4 + 3 + 6 + 8 + 9 + 4 = 42, {} = 2 => tong 80+24+42+2 < 200

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
//TaskHandle_t xRxTask;
//TaskHandle_t xTxTask;

void init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_2, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void step_error(void){
	stepError++;
	if (stepError > 15) {stepError = 0; step = 115; }
}

void get_string_between_commas(char *str, int n, char *result) {
	int num_commas = 0; // number of comma
	int commas[30]; // store comma position
	// count comma
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == ',') {
            commas[num_commas] = i; // store position
            num_commas++; // number of comma++
            if (num_commas > n) {break;}
        }
    } //toc do 0(strlen(str)) aka kha cham
    if (num_commas == 0 || num_commas < n || n < 0) {
    	strcpy(result, "NONE");
    	return;
    }
    else if ( (n == num_commas && strlen(str) - commas[n-1] - 1 == 0) || (num_commas > n && n > 0 && commas[n] - commas[n-1] - 1  == 0) || (n == 0 && commas[0] == 0) ) {
    	strcpy(result, "NONE");
    	return;
    }
    if (n == 0){
    	strncpy(result, str, commas[0]);
		result[commas[0]] = '\0';
    }
    else if (n == num_commas){
    	strncpy(result, str + commas[n-1] + 1, strlen(str) - commas[n-1] - 1);
    	result[strlen(str) - commas[n-1] - 1] = '\0';
    }
    else {
    	strncpy(result, str + commas[n-1]+1, commas[n] - commas[n-1] - 1);
    	result[commas[n] - commas[n-1] - 1] = '\0';
    }
}

int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_2, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

static void tx_task(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1) {
    	if(flag == 1){
    		flag = 0;
			switch(step){
				case 1: //AT command
					sendData(TX_TASK_TAG, "AT\r");
					ESP_LOGI(TX_TASK_TAG,"AT");
					break;
				case 2: //AT IPR baud rate
					sendData(TX_TASK_TAG, "AT+IPR=115200\r");
					ESP_LOGI(TX_TASK_TAG,"AT+IPR=115200");
					break;
				case 3: //GNSS on (because it's gonna take a long time)
					sendData(TX_TASK_TAG, "AT+CGNSPWR=1\r");
					ESP_LOGI(TX_TASK_TAG,"AT+CGNSPWR=1");
					break;
				case 4: //Double check APP
					sendData(TX_TASK_TAG, "AT+CNACT?\r");
					ESP_LOGI(TX_TASK_TAG,"AT+CNACT?");
					break;
				case 5: //IP Application
					sendData(TX_TASK_TAG, "AT+CNACT=0,1\r");
					ESP_LOGI(TX_TASK_TAG,"AT+CNACT=0,1");
					break;
				case 6: //AT+SMCONF="CLIENTID","DinhviESP32" //Start config mqtt
					sendData(TX_TASK_TAG, "AT+SMCONF=\"CLIENTID\",\"DinhviESP32\"\r");
					ESP_LOGI(TX_TASK_TAG,"AT+SMCONF=\"CLIENTID\",\"DinhviESP32\"");
					break;
				case 7: //AT+SMCONF="URL","mqtt.innoway.vn",1883
					sendData(TX_TASK_TAG, "AT+SMCONF=\"URL\",\"mqtt.innoway.vn\",1883\r");
					ESP_LOGI(TX_TASK_TAG,"AT+SMCONF=\"URL\",\"mqtt.innoway.vn\",1883");
					break;
				case 8://AT+SMCONF="KEEPTIME",60
					sendData(TX_TASK_TAG, "AT+SMCONF=\"KEEPTIME\",60\r");
					ESP_LOGI(TX_TASK_TAG,"AT+SMCONF=\"KEEPTIME\",60");
					break;
				case 9: //AT+SMCONF="USERNAME","QToan"
					sendData(TX_TASK_TAG, "AT+SMCONF=\"USERNAME\",\"QToan\"\r");
					ESP_LOGI(TX_TASK_TAG,"AT+SMCONF=\"USERNAME\",\"QToan\"");
					break;
				case 10: //AT+SMCONF="PASSWORD","jlOeIr4M0REGYsmNMNajxUpxKlmdPDR1"
					sendData(TX_TASK_TAG, "AT+SMCONF=\"PASSWORD\",\"jlOeIr4M0REGYsmNMNajxUpxKlmdPDR1\"\r");
					ESP_LOGI(TX_TASK_TAG,"AT+SMCONF=\"PASSWORD\",\"jlOeIr4M0REGYsmNMNajxUpxKlmdPDR1\"");
					break;
				case 11: //AT+SMCONF="CLEANSS",1
					sendData(TX_TASK_TAG, "AT+SMCONF=\"CLEANSS\",1\r");
					ESP_LOGI(TX_TASK_TAG,"AT+SMCONF=\"CLEANSS\",1");
					break;
				case 12: //AT+SMCONF="QOS",1
					sendData(TX_TASK_TAG, "AT+SMCONF=\"QOS\",1\r");
					ESP_LOGI(TX_TASK_TAG,"AT+SMCONF=\"QOS\",1");
					break;
				case 13: //AT+SMCONF?
					sendData(TX_TASK_TAG, "AT+SMCONF?\r");
					ESP_LOGI(TX_TASK_TAG,"AT+SMCONF?");
					break;
				case 14: //AT+SMCONN
					sendData(TX_TASK_TAG, "AT+SMCONN\r");
					ESP_LOGI(TX_TASK_TAG,"AT+SMCONN");
					break;

				case 15: //AT+SMSUB="messages/681b35dc-8a93-4f52-a51d-7710d30e4bdf/status",0
					sendData(TX_TASK_TAG, "AT+SMSUB=\"messages/681b35dc-8a93-4f52-a51d-7710d30e4bdf/status\",0\r");
					ESP_LOGI(TX_TASK_TAG,"AT+SMSUB=\"messages/681b35dc-8a93-4f52-a51d-7710d30e4bdf/status\",0");
					break;

				// Update chu ki:
				case 16: // Check ket noi MQTT
					sendData(TX_TASK_TAG, "AT+SMSTATE?\r");
					ESP_LOGI(TX_TASK_TAG,"AT+SMSTATE?");
					break;
				case 17: //Lay thong tin RSRP, RSRQ, SINR, PCI, CellID cot dau tien (cung co the dung AT+CPSI? hien tai nhung khong co pci)
					sendData(TX_TASK_TAG, "AT+CENG?\r");
					ESP_LOGI(TX_TASK_TAG,"AT+CENG?");
					break;
				case 18: //Lay thong tin thoi gian & kinh do, vi do
					sendData(TX_TASK_TAG, "AT+CGNSINF\r");
					ESP_LOGI(TX_TASK_TAG,"AT+CGNSINF");
					break;
				case 19: //AT+SMPUB="messages/681b35dc-8a93-4f52-a51d-7710d30e4bdf/update",?,0,1    > {"st":0}
					snprintf(temp, sizeof(temp), "AT+SMPUB=\"messages/681b35dc-8a93-4f52-a51d-7710d30e4bdf/update\",%d,0,1\r", dataSendSize);
					sendData(TX_TASK_TAG, temp);
					ESP_LOGI(TX_TASK_TAG,"AT+SMPUB=\"messages/681b35dc-8a93-4f52-a51d-7710d30e4bdf/update\",%d,0,1",dataSendSize);
					break;
				case 20: // ban tin pub
					sendData(TX_TASK_TAG, dataSend);
					ESP_LOGI(TX_TASK_TAG,"Gui");
					break;
//				case 21: //AT+SMUNSUB
//					sendData(TX_TASK_TAG, "AT+SMUNSUB=\"messages/681b35dc-8a93-4f52-a51d-7710d30e4bdf/status\"\r");
//					ESP_LOGI(TX_TASK_TAG,"AT+SMUNSUB=\"messages/681b35dc-8a93-4f52-a51d-7710d30e4bdf/status\",0");
//					break;

				//reset things:
				case 115:
					sendData(TX_TASK_TAG, "AT+CREBOOT\r");
					ESP_LOGI(TX_TASK_TAG,"AT+CREBOOT");
					break;

				default:
					break;
			}
			//vTaskResume(xRxTask);
			//vTaskSuspend(xTxTask);
    	}
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_2, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
        if (rxBytes > 0) {
			data[rxBytes] = 0;
			ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
        	switch(step){
				case 4: //check APP
					if (strstr((char*)data, "+CNACT: 0,1,") != NULL) { step = 6; }
                    else { step = 5; }
					flag = 1;
					break;
				case 5: //check APP
                    if (strstr((char*)data, "DEACTIVE") == NULL && strstr((char*)data, "ACTIVE") != NULL) {
                    	step = 4;
                    	stepError = 0;
                    }
                    else if (strstr((char*)data, "DEACTIVE") == NULL){ step_error(); }
                    flag = 1;
					break;
				case 16: //check ket noi mqtt
					if (strstr((char*)data, "+SMSTATE: 0") == NULL){ step++; }
					else { step = 14; }
					flag = 1;
					break;
				case 17: //Lay thong tin RSRP, RSRQ, SINR, PCI, CellID cot dau tien (cung co the dung AT+CPSI? hien tai nhung khong co pci)
					int cellNumber = 0;
					char *cellData = strstr((char *)data, "+CENG: 1,1,");
					sscanf(cellData, "+CENG: 1,1,%d,%s", &cellNumber, temp);
					if (cellNumber > 0){
						char *cellDataNo0 = strstr((char *)cellData, "+CENG: 0,\"");
						sscanf(cellDataNo0, "+CENG: 0,\"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &others, &pci, &rsrp, &others, &rsrq, &sinr, &others, &cellID, &others, &others, &others);
						//get_string_between_commas((char*)data, 3, PCI);
						//get_string_between_commas((char*)data, 4, RSRP);
						//get_string_between_commas((char*)data, 6, RSRQ);
						//get_string_between_commas((char*)data, 7, SINR);
						//get_string_between_commas((char*)data, 9, cellID);
						stepError = 0;
						step++;
					}
					else { step_error(); }
					flag = 1;
					break;
				case 18: //Lay thong tin thoi gian & kinh do, vi do
					get_string_between_commas((char*)data, 3, temp);
					if (strcmp(temp,"NONE")){
						get_string_between_commas((char*)data, 3, temp);
						date = strtof(temp, NULL);
						get_string_between_commas((char*)data, 4, temp);
						latitude = strtof(temp, NULL);
						get_string_between_commas((char*)data, 5, temp);
						longitude = strtof(temp, NULL);
						stepError = 0;
						step++;
						sprintf(temp, "{\\\"RSRP\\\":%d,\\\"RSRQ\\\":%d,\\\"SINR\\\":%d,\\\"PCI\\\":%d,\\\"cellID\\\":%d,\\\"latitude\\\":%0.6f,\\\"longitude\\\":%0.6f,\\\"date\\\":%0.0f}\\\r", rsrp, rsrq, sinr, pci, cellID, latitude, longitude, date);
						ESP_LOGI(RX_TASK_TAG, "We have: '%s'", temp);
						dataSendSize = strlen(temp) - 17; // dau \ xuat hien truoc ", co 8 data, suy ra 16 dau \ ummm va 1 dau truoc \r
						strcpy(dataSend, temp);
					}
					else { step_error(); }
					flag = 1;
					break;
				case 20: //luc gui ban tin:
					if (strstr((char*)data, "+SMSUB") != NULL){
    					vTaskDelay(60000*5 / portTICK_PERIOD_MS);
    					step = 16;
    					flag = 1;
        			}
					break;

				case 115:
					if (strstr((char*)data, "RDY") != NULL){
						step = 1;
						vTaskDelay(20000 / portTICK_PERIOD_MS);
						flag = 1;
        			}
					break;

        		default:
                    if (strstr((char*)data, "OK") != NULL) {
                    	step++;
                    	stepError = 0;
                    }
					if (strstr((char*)data, "ERROR") != NULL) {
						step_error();
					}
					flag = 1;
                    //vTaskResume(xTxTask);
                    //vTaskSuspend(xRxTask);
        			break;
        	}
        }
    }
    free(data);
}

void app_main(void)
{
    init();
    xTaskCreate(rx_task, "uart_rx_task", 1024*4, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024*4, NULL, configMAX_PRIORITIES-1, NULL);
}
