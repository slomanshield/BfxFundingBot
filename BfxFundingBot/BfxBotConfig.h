#pragma once
#include "global.h"
class BfxBotConfig
{
	public:
		BfxBotConfig();
		~BfxBotConfig();
		int Init(char* pathToConfig);
		int ExtractConfigData();
		int GetLogicTime();
		void GetApiKey(std::string* output);
		void GetSecretKey(std::string* output);
		void GetSendToEmail(std::string* output);
		void GetEmailUser(std::string* output);
		void GetEmailPass(std::string* output);
		int GetOrderBookMultiplier();
		double GetRetryReducePercent();
		int GetRetry();
	private:
		int logicTimer;
		std::string apiKey;
		std::string secretKey;
		std::string toEmail;
		std::string emailUser;
		std::string emailPass;
		int orderBookMultiplier;
		double retryReducePercent;
		int retryAmount;
		Document doc;
};