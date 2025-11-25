#include "NetlistUnit.h"

void NetlistUnit::AddPin(const std::string& synbolName, const std::string& pinD, const std::string& pinG, const std::string& pinS)
{
	this->pinMap[synbolName] = { pinD, pinG, pinS };
}

void NetlistUnit::AddPinD(const std::string& synbolName, const std::string& pinD)
{
	if (this->pinMap.find(synbolName) == this->pinMap.end())
	{
		this->pinMap[synbolName] = std::vector<std::string>(3, "");
	}
	this->pinMap[synbolName][0] = pinD;
}

void NetlistUnit::AddPinG(const std::string& synbolName, const std::string& pinG)
{
	if (this->pinMap.find(synbolName) == this->pinMap.end())
	{
		this->pinMap[synbolName] = std::vector<std::string>(3, "");
	}
	this->pinMap[synbolName][1] = pinG;
}

void NetlistUnit::AddPinS(const std::string& synbolName, const std::string& pinS)
{
	if (this->pinMap.find(synbolName) == this->pinMap.end())
	{
		this->pinMap[synbolName] = std::vector<std::string>(3, "");
	}
	this->pinMap[synbolName][2] = pinS;
}




std::vector<std::string> NetlistUnit::GetPins(const std::string& synbolName) const
{
	auto it = this->pinMap.find(synbolName);
	if (it != this->pinMap.end())
	{
		return it->second;
	}
	return {};
}

std::string NetlistUnit::GetPinD(const std::string& synbolName) const
{
	auto it = this->pinMap.find(synbolName);
	if (it != this->pinMap.end())
	{
		return it->second[0];
	}
	return "";
}

std::string NetlistUnit::GetPinG(const std::string& synbolName) const
{
	auto it = this->pinMap.find(synbolName);
	if (it != this->pinMap.end())
	{
		return it->second[1];
	}
	return "";
}

std::string NetlistUnit::GetPinS(const std::string& synbolName) const
{
	auto it = this->pinMap.find(synbolName);
	if (it != this->pinMap.end())
	{
		return it->second[2];
	}
	return "";
}