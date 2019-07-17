#include "BfxFundingBotMain.h"

BfxFundingBotMain::BfxFundingBotMain()
{
	currTime = 0;
	lastTimeProcessed = 0;
	currTimeWalletCheck = 0;
	lastTryReconnect = 0;
	currency = "fUSD";
}

BfxFundingBotMain::~BfxFundingBotMain()
{
}

int BfxFundingBotMain::Init(char* pathToConfig)
{
	int cc = 0;;
	cc = bfxBotConfig.Init(pathToConfig);
	if (cc == SUCCESS)
	{
		bfxBotConfig.GetEmailUser(&emailUser);
		bfxBotConfig.GetEmailPass(&emailPass);
		processIntervalSecs = bfxBotConfig.GetLogicTime();
		retryReducePercent = bfxBotConfig.GetRetryReducePercent();
		numRetriesAllowed = bfxBotConfig.GetRetry();
		orderBookMultiplier = bfxBotConfig.GetOrderBookMultiplier();
	}
	if (cc == SUCCESS)
	{
		bfxBotConfig.GetApiKey(&apiKey);
		bfxBotConfig.GetSecretKey(&secretKey);
		bfxLibrary.InitKeys((char*)apiKey.c_str(), (char*)secretKey.c_str());
	}
	
	return cc;
}

int BfxFundingBotMain::SendEmail(std::string to,std::string to_addr, std::string message, std::string subject)
{
	int cc = SUCCESS;
	stringdata payload = GenerateEmailPayload(to,to_addr, message, subject);
	CURL *curl;
	struct curl_slist *recipients = NULL;
	CURLcode res = CURLE_OK;
	curl = curl_easy_init();

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, GOOGLE_SMTP);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
		curl_easy_setopt(curl, CURLOPT_PORT, 465);
		curl_easy_setopt(curl, CURLOPT_USERNAME, emailUser.c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, emailPass.c_str());

		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM_EMAIL);

		recipients = curl_slist_append(recipients, to_addr.c_str());
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, &payload);

		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		res = curl_easy_perform(curl);

		if (res != CURLE_OK)
		{
			LOG_ERROR(string_format(std::string("Error sending email: %s"), curl_easy_strerror(res)));
			cc = ErrorSendingEamil;
		}

		curl_slist_free_all(recipients);
		curl_easy_cleanup(curl);
	}


	return cc;
}

int BfxFundingBotMain::SendCurrentErrorFile()
{
	int cc = SUCCESS;
	std::string pathToLog;
	fstream fileHandle;
	time_t now = time(0);
	struct tm* ptm = localtime(&now);
	char buf[] = "YYYYMMDD.log";

	snprintf(buf, sizeof(buf), "%04d%02d%02d.log", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
	bfxBotConfig.GetSendToEmail(&toEmail);

	fileHandle.open(std::string(std::string(LOG_PATH) + "/" + buf).c_str(), std::fstream::in);
	if (fileHandle.is_open())
	{
		fileHandle.seekg(0, fileHandle.end);
		unsigned long  fileLength = fileHandle.tellg();
		if (fileLength > 0)
		{
			char* buffer = new char[fileLength];
			memset(buffer, 0, fileLength);
			fileHandle.seekg(0, fileHandle.beg);

			fileHandle.read(buffer, fileLength);

			cc = SendEmail("User", toEmail, std::string(buffer, fileLength), std::string("ERROR LOG"));

			delete[] buffer;
		}
		
		fileHandle.close();
	}

	return cc;
}

int BfxFundingBotMain::Process()
{
	int cc = SUCCESS;
	if (bfxLibrary.GetWebSocketStatus() != WebSocket_ReadyState_Open)
	{
		cc = WebSocketConnectFailure; // since we arent connected just set it to failure so we do not log
		if (ErrorConnectingBfx == false)
			ErrorConnectingBfx = true;
		if(IsTimeToReconnect())
			cc = ReConnectWebSocket();
		if (cc == SUCCESS)
			ErrorConnectingBfx = false;
	}

	if (TimeToModify() == true && ErrorConnectingBfx == false)
	{

		if (cc == SUCCESS)
		{
			cc = ProcessWrapper(modify_offers);
			
		}
	}

	if (TimeToCheckWallet() == true && ErrorConnectingBfx == false)
	{
		if (cc == SUCCESS)
		{
			cc = ProcessWrapper(post_offers);
		}
	}

	return cc;
}

int BfxFundingBotMain::ModifyExistingOffers()
{
	int cc = SUCCESS;
	int offer_id = 0;
	double rate = 0;
	double amount;
	Document doc;
	Document doc_2;

	cc = bfxLibrary.PostGetAllCurrentOffers(&doc);

	if (cc == SUCCESS)
	{
		if (doc.IsArray())
		{
			for (int i = 0; i < doc.Size(); i++)
			{
				Value& val = doc[i];

				if (val.IsObject())
				{
					offer_id = val["id"].GetInt();
					rate = convert_365_rate_to_decimal(strtod(val["rate"].GetString(),NULL)); // get true decimal value, its needed for the post call
					modf(strtod(val["remaining_amount"].GetString(),NULL), &amount);//get whole number

					cc = bfxLibrary.PostCancelOffer(offer_id, &doc_2);
					
					if (cc == SUCCESS)
					{
						rate = rate - (rate * (retryReducePercent * .01));// percent is a whole number
						cc = bfxLibrary.PostFundingOffer(&currency, amount, rate, 2, &doc_2);
					}
					
				}
				else
					cc = MALFORMED_JSON_FROM_BFX;
			}
		}
		else
			cc = MALFORMED_JSON_FROM_BFX;
	}

	return cc;
}

int BfxFundingBotMain::PostOffer()
{
	int cc = SUCCESS;
	Document doc;
	double avaiableAmount = GetAvaiableAmount(&cc);

	if (cc == SUCCESS and avaiableAmount >= MINIMUM_OFFER_AMOUNT)
	{
		double rate = GetRateForPost(&cc, avaiableAmount);

		if (cc == SUCCESS && rate > 0)
		{
			cc = bfxLibrary.PostFundingOffer(&currency, avaiableAmount, rate, 2, &doc);
		}
		if (rate == -1)
		{
			cc = ErrorFindingBestRate;
		}
	}

	return cc;
}

int BfxFundingBotMain::ReConnectWebSocket()
{
	int cc = SUCCESS;
	bfxLibrary.DisconnetWebSocket();
	cc = bfxLibrary.ConnectWebSocket();
	if (cc == SUCCESS)
	{
		cc = bfxLibrary.SendAuthWebSocket();
	}
	else
		LOG_ERROR_CODE(cc);

	return cc;
}

int BfxFundingBotMain::ProcessWrapper(int type) // we use this function if we need to reconenct cause BFX hates us
{
	int cc = SUCCESS;
	int numTries = 0;

	do
	{
		switch (type)
		{
			case modify_offers:
			{
				cc = ModifyExistingOffers();
				break;
			}

			case post_offers:
			{
				cc = PostOffer();
				break;
			}
		}

		if (numTries < numRetriesAllowed)
		{
			if (cc == WebSocketGetDataTimeout || cc == WebSocketNotConnected)
			{
				cc = ReConnectWebSocket();

				if (cc == SUCCESS)
				{
					cc = WebSocketGetDataTimeout;
				}

			}
		}
		
		numTries++;
	} while ( (cc == WebSocketGetDataTimeout || cc == WebSocketNotConnected) && numTries < numRetriesAllowed);

	return cc;
}

double BfxFundingBotMain::GetAvaiableAmount(int* cc)
{
	double avaiableAmount = -1;
	std::string type;
	Document doc;

	*cc = bfxLibrary.PostGetMarginAvailableFunding(&doc);

	if (*cc == SUCCESS)
	{
		if (doc.IsArray())
		{
			for (int i = 0; i < doc.Size(); i++)
			{
				Value& object = doc[i];
				type = object["type"].GetString();
				if (type.compare("deposit") == SUCCESS)
				{
					avaiableAmount = strtod(object["available"].GetString(),NULL);
					break;
				}
			}
		}
		else
			*cc = MALFORMED_JSON_FROM_BFX;
	}

	if(avaiableAmount == -1 && *cc == SUCCESS)//we got something back but it was unexpected
	{
		std::string json;
		GetJsonString(&doc, &json);
		LOG_ERROR(string_format(std::string("Error getting wallet amount, json is: %s"), json).c_str());
		*cc = MALFORMED_JSON_FROM_BFX;
	}

	return avaiableAmount;
}

double BfxFundingBotMain::GetRateForPost(int * cc,double amountAvaiable)
{
	double rate = -1;
	double amountLimit = GetAmountLimit(amountAvaiable);
	double currentAmount = 0;
	Document doc;

	*cc = bfxLibrary.GetFundingOrderBook(&doc, ORDER_BOOK_DEPTH);

	if (*cc == SUCCESS)
	{
		if (doc.IsObject())
		{
			if (doc["asks"].IsArray())
			{
				Value askArray = doc["asks"].GetArray();

				for (int i = 0; i < askArray.Size(); i++)
				{
					Value& obj = askArray[i];
					currentAmount += strtod(obj["amount"].GetString(), NULL);

					if (currentAmount >= amountLimit || i == askArray.Size() - 1)//if we make it here cause of reaching the end then damnnnnn lol
					{
						rate = convert_365_rate_to_decimal(strtod(obj["rate"].GetString(), NULL)) - BEST_RATE_REDUCE_AMOUNT;
						break;
					}
					
				}
			}
			else
				*cc = MALFORMED_JSON_FROM_BFX;

		}
		else
			*cc = MALFORMED_JSON_FROM_BFX;
	}

	return rate;
}

double BfxFundingBotMain::GetAmountLimit(double amountAvaiable)
{
	int numWholeDigits = GetWholeNumAmount(amountAvaiable);

	if (numWholeDigits < MAX_SIG_WHOLE_DIGITS)
	{
		int digitDelta = MAX_SIG_WHOLE_DIGITS - numWholeDigits;
		return amountAvaiable * (orderBookMultiplier / (1 / (pow(10, digitDelta) )));
	}
	else if (numWholeDigits >= MAX_SIG_WHOLE_DIGITS)
	{
		return amountAvaiable * orderBookMultiplier;
	}
}



std::string BfxFundingBotMain::GenerateEmailMessageId()
{
	const int MESSAGE_ID_LEN = 37;
	srand(GET_NONCE);

	time_t t;
	

	std::string ret;
	ret.resize(15);

	time(&t);
	struct tm* tm = gmtime(&t);

	strftime(const_cast<char *>(ret.c_str()),
		MESSAGE_ID_LEN,
		"%Y%m%d%H%M%S.",
		tm);

	ret.reserve(MESSAGE_ID_LEN);

	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	while (ret.size() < MESSAGE_ID_LEN) {
		ret += alphanum[rand() % (sizeof(alphanum) - 1)];
	}


	return ret;
}

std::string BfxFundingBotMain::GenerateEmailPayload(std::string to, std::string to_addr, std::string message, std::string subject)
{
	std::string payloadTemplate = "Date: %s \r\n"
						  "To: %s <%s> \r\n"
						  "From: %s <%s> \r\n"
						  "Cc: <> \r\n"
						  "Message-ID: <%s@%s> \r\n"
						  "Subject: %s \r\n"
						  "\r\n"
						  "%s \r\n"
						  "\r\n"
						  "\r\n"
						  "\r\n";

	std::string fromEmail = FROM_EMAIL;

	std::string payload = string_format(payloadTemplate, dateTimeNow().c_str(), to.c_str(), to_addr.c_str(), FROM_NAME, fromEmail.c_str(), GenerateEmailMessageId().c_str(),
										fromEmail.substr(fromEmail.find('@') + 1).c_str(), subject.c_str(), message.c_str());

	return payload;
}

bool BfxFundingBotMain::TimeToModify()
{
	if ((currTime - lastTimeProcessed >= processIntervalSecs) || currTime == 0)
	{
		lastTimeProcessed = time(NULL);
		currTime = lastTimeProcessed;
		return true;
	}
		
	currTime = time(NULL);
	return false;
}

bool BfxFundingBotMain::TimeToCheckWallet()
{
	if ((currTimeWalletCheck - lastTimeWalletCheck >= CheckWalletAmountIntervalSecs) || currTimeWalletCheck == 0)
	{
		lastTimeWalletCheck = time(NULL);
		currTimeWalletCheck = lastTimeWalletCheck;
		return true;
	}

	currTimeWalletCheck = time(NULL);
	return false;
}

bool BfxFundingBotMain::IsTimeToReconnect()
{
	time_t currTime = time(NULL);
	if (currTime - lastTryReconnect >= ReconnectTimeoutSecs)
	{
		lastTryReconnect = currTime;
		return true;
	}
	return false;
}
