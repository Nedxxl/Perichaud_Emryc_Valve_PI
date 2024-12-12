#include "task2.hpp"
#include "screen.hpp"

TTask2::TTask2(const char *name, void *shared, int32_t policy, int32_t priority, int32_t cpu) : TThread(name, shared, policy, priority, cpu)
{
	std::string nameMqtt;

	screen = (TScreen *)shared;
	ads1115 = new ADS1115("/dev/i2c-1");

	screen->dispStr(1, 7, "Task 2 (CPU :  ) :");

	nameMqtt = TMqtt::getMqttConfig();
	mqtt = new TMqtt(screen, "mqttValve", nameMqtt.c_str());
}

TTask2::~TTask2()
{
	if (ads1115)
		delete ads1115;

	if (mqtt)
		delete mqtt;
}

void TTask2::task(void)
{
	// variable locale
	char strCar[2] = {'-', '\0'};
	char bufStr[100];
	char bufStrGB[100];
	char bufStrPB[100];
	uint16_t result;
	uint16_t resultGB;
	uint16_t resultPB;
	uint16_t oldResultGb;
	uint16_t oldResultPb;

	// synchronisation démarrage tâche
	signalStart();

	while (1)
	{
		// traitement
		sprintf(bufStr, "%d", sched_getcpu());
		screen->dispStr(15, 7, bufStr);
		if (strCar[0] == '|')
			strCar[0] = '-';
		else
			strCar[0] = '|';
		screen->dispStr(20, 7, strCar);

		result = ads1115->readChannelADC(1);
		sprintf(bufStr, "Circuit 1 GB (Canal 1) : %04X %07d %0.2f %%", result, result, ((result / 28000.0) * 90));
		screen->dispStr(1, 17, bufStr);
		resultGB = (float)((result / 28000.0) * 90);
		if (resultGB < 0)
			resultGB = 0;
		else if (resultGB > 90)
			resultGB = 90;
		sprintf(bufStrGB, "%d", resultGB);

		if (resultGB != oldResultGb)
		{
			mqtt->publish(NULL, "RAM/valves/etats/Ouverture_GB", strlen(bufStrGB), bufStrGB);
			oldResultGb = resultGB;
		}

		result = ads1115->readChannelADC(0);
		sprintf(bufStr, "Circuit 2 LB (Canal 0) : %04X %07d %0.2f %%", result, result, ((result / 28000.0) * 90));
		screen->dispStr(1, 18, bufStr);
		resultPB = (float)((result / 28000.0) * 90);
		if (resultPB < 0)
			resultPB = 0;
		else if (resultPB > 90)
			resultPB = 90;
		sprintf(bufStrPB, "%d", resultPB);

		if (resultPB != oldResultPb)
		{
			mqtt->publish(NULL, "RAM/valves/etats/Ouverture_PB", strlen(bufStrPB), bufStrPB);
			oldResultPb = resultPB;
		}

		usleep(500000); // 500 ms
	}
}
