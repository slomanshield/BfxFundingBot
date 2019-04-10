#include "BfxBotConfig.h"

BfxBotConfig::BfxBotConfig()
{
	logicTimer = -1;
	apiKey = "";
	secretKey = "";
	orderBookMultiplier = -1;
	retryReducePercent = -1;
}

BfxBotConfig::~BfxBotConfig()
{
	return;//do nothing
}

int BfxBotConfig::Init(char* pathToConfig)
{
	int cc = SUCCESS;
	fstream fileHandle;
	fileHandle.open(pathToConfig, std::fstream::in);
	if (fileHandle.is_open())
	{

		fileHandle.seekg(0, fileHandle.end);
		unsigned long  fileLength = fileHandle.tellg();
		char* buffer = new char[fileLength];
		memset(buffer, 0, fileLength);
		fileHandle.seekg(0, fileHandle.beg);

		fileHandle.read(buffer, fileLength);

		if (doc.IsObject())
			doc.GetAllocator().Clear();
		ParseResult parseResult = doc.Parse(buffer, fileLength);

		if (parseResult.IsError() == false)
		{
			cc = ExtractConfigData();
		}
		else
			cc = INVALID_CONFIG_DATA;

		delete[] buffer;
		fileHandle.close();
	}
	else
		cc = CONFIG_NOT_FOUND;

	return cc;
}

int BfxBotConfig::ExtractConfigData()
{
	int cc = SUCCESS;

	do
	{
		if (doc.HasMember("Logic Timer") && doc["Logic Timer"].IsInt())
		{
			logicTimer = doc["Logic Timer"].GetInt();
		}
		else
		{
			LOG_ERROR("Invalid Logic Timer Value");
			cc = INVALID_CONFIG_VALUE;
			break;
		}


		if (cc == SUCCESS && doc.HasMember("Api-Key") && doc["Api-Key"].IsString())
		{
			apiKey = doc["Api-Key"].GetString();
		}
		else
		{
			LOG_ERROR("Invalid Api-Key Value");
			cc = INVALID_CONFIG_VALUE;
			break;
		}

		if (cc == SUCCESS && doc.HasMember("Secret-Key") && doc["Secret-Key"].IsString())
		{
			secretKey = doc["Secret-Key"].GetString();
		}
		else
		{
			LOG_ERROR("Invalid Secret-Key Value");
			cc = INVALID_CONFIG_VALUE;
			break;
		}

		if (cc == SUCCESS && doc.HasMember("Order Book Multiplier") && doc["Order Book Multiplier"].IsInt())
		{
			orderBookMultiplier = doc["Order Book Multiplier"].GetInt();
		}
		else
		{
			LOG_ERROR("Invalid Order Book Multiplier Value");
			cc = INVALID_CONFIG_VALUE;
			break;
		}

		if (cc == SUCCESS && doc.HasMember("Retry Reduce Percent") && doc["Retry Reduce Percent"].IsDouble())
		{
			retryReducePercent = doc["Retry Reduce Percent"].GetDouble() / 100;//get actual decimal percent
		}
		else
		{
			LOG_ERROR("Invalid Retry Reduce Percent Value");
			cc = INVALID_CONFIG_VALUE;
			break;
		}

		if (cc == SUCCESS && doc.HasMember("Retry Amount") && doc["Retry Amount"].IsInt())
		{
			retryAmount = doc["Retry Amount"].GetInt();
		}
		else
		{
			LOG_ERROR("Invalid Retry Amount Value");
			cc = INVALID_CONFIG_VALUE;
			break;
		}

		if (cc == SUCCESS && doc.HasMember("Send To Email") && doc["Send To Email"].IsString())
		{
			toEmail = doc["Send To Email"].GetString();
		}
		else
		{
			LOG_ERROR("Invalid Email Value");
			cc = INVALID_CONFIG_VALUE;
			break;
		}

		if (cc == SUCCESS && doc.HasMember("Email User") && doc["Email User"].IsString())
		{
			emailUser = doc["Email User"].GetString();
		}
		else
		{
			LOG_ERROR("Invalid Email User Value");
			cc = INVALID_CONFIG_VALUE;
			break;
		}

		if (cc == SUCCESS && doc.HasMember("Email Pass") && doc["Email Pass"].IsString())
		{
			emailPass = doc["Email Pass"].GetString();
		}
		else
		{
			LOG_ERROR("Invalid Email Pass Value");
			cc = INVALID_CONFIG_VALUE;
			break;
		}
	} while (false);
	

	return cc;
}

int BfxBotConfig::GetLogicTime()
{
	return logicTimer;
}

void BfxBotConfig::GetApiKey(std::string* output)
{
	*output = apiKey;
	return;
}

void BfxBotConfig::GetSecretKey(std::string* output)
{
	*output = secretKey;
	return;
}

void BfxBotConfig::GetSendToEmail(std::string * output)
{
	*output = toEmail;
	return;
}

void BfxBotConfig::GetEmailUser(std::string * output)
{
	*output = emailUser;
	return;
}

void BfxBotConfig::GetEmailPass(std::string * output)
{
	*output = emailPass;
	return;
}

int BfxBotConfig::GetOrderBookMultiplier()
{
	return orderBookMultiplier;
}

double BfxBotConfig::GetRetryReducePercent()
{
	return retryReducePercent;
}
int BfxBotConfig::GetRetry()
{
	return retryAmount;
}