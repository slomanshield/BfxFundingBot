
#include "global.h"
#include "BfxBotConfig.h"
#include "BfxLibrary.h"
#include "BfxFundingBotMain.h"


void sig_term_handler(int signum)
{
	if (signum == SIGTERM || signum == SIGQUIT || signum == SIGKILL)
		TerminateApplication = true;
}

int main(int argc, char** argv)
{
	struct sigaction new_action, old_action;
 
	new_action.sa_handler = sig_term_handler;
	sigemptyset(&new_action.sa_mask);
	new_action.sa_flags = 0;

	
	sigaction(SIGTERM, &new_action, NULL);
	
	sigaction(SIGQUIT, &new_action, NULL);
	
	sigaction(SIGKILL, &new_action, NULL);

	Logger::getInstance()->setLevel(Logger::LEVEL_TRACE);
	//BfxBotConfig bfxBotConfig;
	//BfxLibrary bfxLibrary;
	BfxFundingBotMain bfxMain;
	int cc = SUCCESS;
	int totalRetries = 0;
	cc = curl_global_init(CURL_GLOBAL_ALL);
	/*std::string apiKey;
	std::string secretKey;
	std::string currency = "fUSD";
	std::string toEmail;

	Document fundingBookData;
	Document balanceData;
	Document liveOffers;
	Document returnData;
	double avaiableAmountDb;*/
	

	//bfxBotConfig.Init("/home/pi/bfx_funding_config");
	
	/*bfxBotConfig.GetApiKey(&apiKey);
	bfxBotConfig.GetSecretKey(&secretKey);
	bfxBotConfig.GetSendToEmail(&toEmail);
	bfxLibrary.InitKeys((char*)apiKey.c_str(), (char*)secretKey.c_str());*/

	//bfxLibrary.GetFundingOrderBook(&fundingBookData, 100);

	//bfxMain.SendCurrentErrorFile();

	if(argc > 1)
		cc = bfxMain.Init(argv[1]);
	else
	{
		cc = ErrorNumParams;
		LOG_ERROR_CODE(cc);
	}

	

	if (cc == SUCCESS)
	{
		while ((cc == SUCCESS || totalRetries < bfxMain.numRetriesAllowed) && TerminateApplication == false)
		{
			cc = bfxMain.Process();
			if (cc != SUCCESS)
				totalRetries++;
			usleep(10000);//sleep for 10 milliseconds
		}
	}
	

	/*bfxLibrary.PostGetMarginAvailableFunding(&balanceData);

	for (int i = 0; i < balanceData.Size(); i++)
	{
		Value& object = balanceData[i];
		std::string type;
		std::string avaiableAmount;

		type = object["type"].GetString();
		if (type.compare("deposit") == SUCCESS)
		{
			avaiableAmount = object["available"].GetString();
			avaiableAmountDb = atof(avaiableAmount.c_str());
			break;
		}
	}*/

	//bfxLibrary.ConnectWebSocket();

	//bfxLibrary.SendAuthWebSocket();

	//bfxLibrary.PostFundingOffer(&currency, avaiableAmountDb, .0003, 2, &returnData);

	//bfxLibrary.PostGetAllCurrentOffers(&liveOffers);//note when reading in the rate divide by 365 then multiply by .01 to get the true decimal rate

	//bfxLibrary.PostCancelOffer(liveOffers[0]["id"].GetInt(), &returnData);

	LOG_INFO("Shutting Down...");


	if (cc != SUCCESS)
	{
		bfxMain.SendCurrentErrorFile();
	}

	curl_global_cleanup();

	return cc;
}