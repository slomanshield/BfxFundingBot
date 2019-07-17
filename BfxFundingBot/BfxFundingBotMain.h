#pragma once
#include "global.h"
#include "BfxBotConfig.h"
#include "BfxLibrary.h"

#define ORDER_BOOK_DEPTH 200
#define MAX_SIG_WHOLE_DIGITS 5
#define BEST_RATE_REDUCE_AMOUNT .00000001
#define MINIMUM_OFFER_AMOUNT 50
#define CheckWalletAmountIntervalSecs 30
#define ReconnectTimeoutSecs 10
class BfxFundingBotMain
{
	public:
		BfxFundingBotMain();
		~BfxFundingBotMain();
		int Init(char* pathToConfig);
		int SendEmail(std::string to, std::string to_addr, std::string message, std::string subject);
		int SendCurrentErrorFile();
		int Process();
		int numRetriesAllowed;
		int ReConnectWebSocket();
	private:
		std::string GenerateEmailMessageId();
		std::string GenerateEmailPayload(std::string to, std::string to_addr, std::string message, std::string subject);
		std::string toEmail;
		std::string emailUser;
		std::string emailPass;
		std::string apiKey;
		std::string secretKey;
		std::string currency;
		BfxBotConfig bfxBotConfig;
		BfxLibrary bfxLibrary;
		time_t currTime;
		time_t lastTimeProcessed;
		time_t currTimeWalletCheck;
		time_t lastTimeWalletCheck;
		time_t lastTryReconnect;

		int processIntervalSecs;
		int orderBookMultiplier;
		double retryReducePercent;

		bool TimeToModify();
		bool TimeToCheckWallet();
		bool IsTimeToReconnect();
		int ModifyExistingOffers();
		double GetAvaiableAmount(int* cc);
		double GetRateForPost(int* cc, double amountAvaiable);
		double GetAmountLimit(double amountAvaiable);
		int PostOffer();

		int ProcessWrapper(int type);
		enum {modify_offers, post_offers};

		bool ErrorConnectingBfx;
};


class stringdata {
public:
	std::string msg;
	size_t bytesleft;

	stringdata(std::string &&m)
		: msg{ m }, bytesleft{ msg.size() }
	{}
	stringdata(std::string &m) = delete;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
	stringdata *text = reinterpret_cast<stringdata *>(userp);

	if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1) || (text->bytesleft == 0)) {
		return 0;
	}

	if ((nmemb * size) >= text->msg.size()) {
		text->bytesleft = 0;
		return text->msg.copy(reinterpret_cast<char *>(ptr), text->msg.size());
	}

	return 0;
}