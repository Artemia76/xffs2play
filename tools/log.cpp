#include "tools/log.h"

namespace xffs2play
{
	CLogger_Event_Handler::CLogger_Event_Handler()
	{
		CLogger::instance()->Subscribe(this);
	}

	CLogger_Event_Handler::~CLogger_Event_Handler()
	{
		CLogger::instance()->UnSubscribe(this);
	}

	CLogger* CLogger::m_ptLogger=nullptr;

	CLogger::CLogger()
	{
		XPLMRegisterFlightLoopCallback(&CLogger::LoopCallback, 1, this);
		thread_id = std::this_thread::get_id();
	}

	CLogger::~CLogger()
	{

	}

	CLogger* CLogger::instance()
	{
		if (m_ptLogger==nullptr)
		{
			m_ptLogger=new CLogger;
		}
		return m_ptLogger;
	}

	void CLogger::kill()
	{
		if (m_ptLogger!=nullptr)
		{
			delete m_ptLogger;
			m_ptLogger=nullptr;
		}
	}

	float CLogger::LoopCallback(float inElapsedSinceLastCall,
									float inElapsedTimeSinceLastFlightLoop,
									int inCounter,
									void* Ref)
	{
		if ((Ref==m_ptLogger) && m_ptLogger)
		{
			m_ptLogger->arrMutex.lock();
			for ( std::string str : m_ptLogger->arrMessages)
			{
				XPLMDebugString(str.c_str());
				m_ptLogger->SendToSubscribers(str);
			}
			m_ptLogger->arrMessages.clear();
			m_ptLogger->arrMutex.unlock();
		}
		return 1;
	}

	void CLogger::log(const std::string& Message, CL_DEBUG_LEVEL pLevel)
	{
		std::string PostMess = "XFFS2Play ";
		switch (pLevel)
		{
			case LEVEL_DEBUG:
			{
				PostMess += "Debug : ";
				break;
			}
			case LEVEL_ERROR:
			{
				PostMess += "Error : ";
				break;
			}
			case LEVEL_INFO:
			{
				PostMess += "Info : ";
				break;
			}
			case LEVEL_WARNING:
			{
				PostMess += "Warning : ";
				break;
			}
		}
		PostMess += Message + '\n';
		if (thread_id == std::this_thread::get_id())
		{
			XPLMDebugString(PostMess.c_str());
			SendToSubscribers(PostMess);
		}
		else
		{
			arrMutex.lock();
			arrMessages.push_back(PostMess);
			while(arrMessages.size() > MAX_BUFFER)
			{
				arrMessages.erase(arrMessages.begin());
			}
			arrMutex.unlock();
		}
	}

	void CLogger::Subscribe(CLogger_Event_Handler* pSubscriber)
	{
		if (std::find(arrSubscribers.begin(), arrSubscribers.end(), pSubscriber)==arrSubscribers.end())
		{
			arrSubscribers.push_back(pSubscriber);
		}
	}

	void CLogger::UnSubscribe(CLogger_Event_Handler* pSubscriber)
	{
		auto it = std::find(arrSubscribers.begin(), arrSubscribers.end(), pSubscriber);
		if (it != arrSubscribers.end())
		{
			arrSubscribers.erase(it);
		}
	}

	void CLogger::SendToSubscribers(const std::string& pMessage)
	{
		for (CLogger_Event_Handler* it : arrSubscribers)
		{
			it->OnLogMessage(pMessage);
		}
	}
}

