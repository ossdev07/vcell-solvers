#include <VCELL/SimulationMessaging.h>
#include <utility>
#include <iostream>
#include <algorithm>
#include <sstream>
using std::cerr;
using std::cout;
using std::endl;

#include <stdio.h>
#ifdef USE_MESSAGING
#if ( !defined(WIN32) && !defined(WIN64) ) // UNIX
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#endif

static const char* MESSAGE_TYPE_PROPERTY	= "MessageType";
static const char* MESSAGE_TYPE_WORKEREVENT_VALUE	= "WorkerEvent";
static const char* MESSAGE_TYPE_STOPSIMULATION_VALUE = "StopSimulation";

static const char* USERNAME_PROPERTY = "UserName";
static const char* HOSTNAME_PROPERTY = "HostName";
static const char* SIMKEY_PROPERTY = "SimKey";
static const char* TASKID_PROPERTY	= "TaskID";
static const char* JOBINDEX_PROPERTY	= "JobIndex";
static const char* WORKEREVENT_STATUS = "WorkerEvent_Status";
static const char* WORKEREVENT_PROGRESS = "WorkerEvent_Progress";
static const char* WORKEREVENT_TIMEPOINT = "WorkerEvent_TimePoint";
static const char* WORKEREVENT_STATUSMSG = "WorkerEvent_StatusMsg";

static const int CONNECTION_RETRY_PERIOD = 10 * ONE_SECOND;
static const int CONNECTION_PING_PERIOD = 30 * ONE_SECOND;
#endif

SimulationMessaging *SimulationMessaging::m_inst = NULL;

SimulationMessaging::SimulationMessaging()
	:events( )
	 {
	bStopRequested = false;
#ifdef USE_MESSAGING
	m_taskID = -1;
	bStarted = false;
#endif
	workerEventOutputMode = WORKEREVENT_OUTPUT_MODE_STDOUT;
}

#ifdef USE_MESSAGING
SimulationMessaging::SimulationMessaging(const char* broker, const char* smqusername, const char* passwd, const char*qname,
		const char* tname, const char* vcusername, int simKey, int jobIndex, int taskID, int ttl_low, int ttl_high)
	:events( )
{
	m_broker = const_cast<char *>(broker);
	m_smqusername = const_cast<char *>( smqusername );
	m_password =  const_cast<char *>(passwd );
	m_qname = const_cast<char *>( qname );
	m_tname = const_cast<char *>( tname );

	m_vcusername = const_cast<char *>( vcusername );
	m_simKey = simKey;
	m_jobIndex = jobIndex;
	m_taskID = taskID;
	
	m_ttl_lowPriority = ttl_low;
	m_ttl_highPriority = ttl_high;

	curl_global_init(CURL_GLOBAL_ALL);

	{
		memset(m_hostname, 0, 256);

		// get hostname
#if ( defined(WIN32) || defined(WIN64) )
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD( 2, 2 );		 
		WSAStartup( wVersionRequested, &wsaData );			
#endif
		gethostname(m_hostname, 256);
		//cout) << "hostname is " << m_hostname << endl;
	}

	time(&lastSentEventTime);

	workerEventOutputMode = WORKEREVENT_OUTPUT_MODE_MESSAGING;

#if ( defined(WIN32) || defined(WIN64) )
    InitializeCriticalSection(&lockForMessaging);
	InitializeCriticalSection(&lockForWorkerEvent);
#else // UNIX
	pthread_mutex_init(&mutex_messaging, NULL);
	pthread_mutex_init(&mutex_workerEvent, NULL);
	pthread_mutex_init(&mutex_cond_workerEvent, NULL);
	pthread_cond_init(&cond_workerEvent, NULL);
	bNewWorkerEvent = false;
#endif

	bStarted = false;
}
#endif

namespace {
	void cleanupWorkerEvent(WorkerEvent *we) {
		delete we;
	}
}

SimulationMessaging::~SimulationMessaging() throw()
{
	if (workerEventOutputMode == WORKEREVENT_OUTPUT_MODE_STDOUT) {
		return;
	}
	std::for_each(events.begin(), events.end( ),cleanupWorkerEvent);
	/*
	if (workerEvent != NULL) {
		delete workerEvent;
		workerEvent = NULL;
	}
	*/
#ifdef USE_MESSAGING
	cleanup();
#if ( defined(WIN32) || defined(WIN64) )
    DeleteCriticalSection(&lockForMessaging);
	DeleteCriticalSection(&lockForWorkerEvent);
#else // UNIX
    pthread_mutex_destroy(&mutex_messaging);
	pthread_mutex_destroy(&mutex_workerEvent);
	pthread_mutex_destroy(&mutex_cond_workerEvent);
	pthread_cond_destroy(&cond_workerEvent);
#endif
	curl_global_cleanup();
#endif
}

SimulationMessaging* SimulationMessaging::getInstVar() {
	return m_inst;
}

SimulationMessaging* SimulationMessaging::create()
{
	if (m_inst == 0){    
        m_inst = new SimulationMessaging();
	}
    return(m_inst);
}

void SimulationMessaging::sendStatus() {
	WorkerEvent * workerEvent = 0; //set to null pointer
	for (;;) {
		{ //scope for locking mutex
			WorkerEventLocker locker(*this);
			if (events.size( ) > 0 ) {
				workerEvent = events.front( );
				events.pop_front( );
			}
			else {
				return;
			}
		} //unlocks mutex

		if (workerEventOutputMode == WORKEREVENT_OUTPUT_MODE_STDOUT) {
			switch (workerEvent->status) {
			case JOB_DATA:
				printf("[[[data:%lg]]]", workerEvent->timepoint);
				fflush(stdout);
				break;
			case JOB_PROGRESS:
				printf("[[[progress:%lg%%]]]", workerEvent->progress * 100.0);
				fflush(stdout);
				break;
			case JOB_STARTING:
				cout<< workerEvent->eventMessage << endl;
				break;
			case JOB_COMPLETED:
				cerr << "Simulation Complete in Main() ... " << endl;
				break;
			case JOB_FAILURE:
				cerr << workerEvent->eventMessage << endl;
				break;
			}
			return;

		} 
#ifdef USE_MESSAGING
		/* get a curl handle */ 
		CURL* curl = curl_easy_init();
		if(curl) {
			// First set the URL that is about to receive our POST. This URL can
			// just as well be a https:// URL if that is what should receive the
			// data.
			curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:6161/message/workerevent?readTimeout=20000&type=queue");


			// Now specify the POST data
			std::stringstream ss;


			//status
			// msg->setIntProperty(WORKEREVENT_STATUS, workerEvent->status);
			ss << WORKEREVENT_STATUS << "=" << workerEvent->status << "&";

			char* revisedMsg = workerEvent->eventMessage;
			if (revisedMsg != NULL) {
				revisedMsg = trim(revisedMsg);
				if (strlen(revisedMsg) > 2048) {
					revisedMsg[2047] = 0; //status message is only 2048 chars long in database
				}

				for (int i = 0; i < (int)strlen(revisedMsg); i ++) {
					switch (revisedMsg[i]) {
					case '\r':
					case '\n':
					case '\'':
					case '\"':
						revisedMsg[i] = ' ';
						break;
						// these characters are not valid both in database and in messages as a property
					}
				}
			}

			//event message
			if (revisedMsg != NULL) {
				//msg->setStringProperty(WORKEREVENT_STATUSMSG, revisedMsg);
				ss << WORKEREVENT_STATUSMSG << "=" << revisedMsg << "&";
			}

			//progress
			// msg->setDoubleProperty(WORKEREVENT_PROGRESS, workerEvent->progress);
			ss << WORKEREVENT_PROGRESS << "=" << workerEvent->progress << "&";

			//timePoint
			// msg->setDoubleProperty(WORKEREVENT_TIMEPOINT, workerEvent->timepoint);
			ss << WORKEREVENT_TIMEPOINT << "=" << workerEvent->timepoint;

			//
			// print message to stdout
			//
			cout << "!!!SimulationMessaging::sendStatus [" << (long)m_simKey << ":" << getStatusString(workerEvent->status);
			if (revisedMsg != NULL) {
				cout << ":" << revisedMsg;
			} else {
				cout << ":" << workerEvent->progress << ":" << workerEvent->timepoint;
			}
			cout << "]" << endl;

			// //send
			// try {
			// 	int timeToLive = m_ttl_highPriority;
			// 	int deliveryMode = DeliveryMode::PERSISTENT;
			// 	switch (workerEvent->status) {
			// 	case JOB_DATA:
			// 		timeToLive = m_ttl_lowPriority;
			// 		break;
			// 	case JOB_PROGRESS:
			// 		timeToLive = m_ttl_lowPriority;
			// 		deliveryMode = DeliveryMode::NON_PERSISTENT;
			// 		break;
			// 	case JOB_STARTING:
			// 		timeToLive = m_ttl_highPriority;
			// 		break;
			// 	case JOB_COMPLETED:
			// 		timeToLive = m_ttl_highPriority;
			// 		break;
			// 	case JOB_FAILURE:
			// 		timeToLive = m_ttl_highPriority;
			// 		break;
			// 	}
			// 	qProducer->send(msg, deliveryMode, DEFAULT_PRIORITY, timeToLive);
			// } catch (CMSException& e) {
			// 	cout << "!!!SimulationMessaging::sendStatus [" << e.getMessage() << "]" << endl;
			// }

			std::string x = ss.str();
			const char* postfields = x.c_str();

			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields); // e.g. postfields="name=daniel&project=curl"
		 
			// Perform the request, res will get the return code
			CURLcode res = curl_easy_perform(curl);
			// Check for errors
			if(res != CURLE_OK)
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		 
			// always cleanup
			curl_easy_cleanup(curl);
		}



		time(&lastSentEventTime);
		if (workerEvent->status == JOB_COMPLETED || workerEvent->status == JOB_FAILURE) {
#if ( defined(WIN32) || defined(WIN64) )
			SetEvent(hMessagingThreadEndEvent);
#else // UNIX
			cout <<  "!!!thread exiting" << endl;
			pthread_exit(NULL);
#endif
		}
#else
			throw "OUPUT_MODE_STANDOUT must be using if not using messaging!";
#endif
		}
		delete workerEvent;
}

void SimulationMessaging::setWorkerEvent(WorkerEvent* arg_workerEvent) {
	if (m_inst == 0) {
		throw "SimulationMessaging is not initialized";
	}
	if (workerEventOutputMode == WORKEREVENT_OUTPUT_MODE_STDOUT) {
		events.push_back(arg_workerEvent);
		sendStatus();
		return;
	} 
#ifdef USE_MESSAGING
	else {
		bool ifset = true;
		bool iftry = false;
		static int lastStatus = -1;
		bool critical = criticalDelivery(*arg_workerEvent);
		if (!critical && arg_workerEvent->status == lastStatus) {
			iftry = true;
			const int dProgress = arg_workerEvent->progress;
			if (dProgress % 25 != 0) {
				time_t currTime = time(0);
				if (currTime - lastSentEventTime < 5) {
					ifset = false;
				}
			}
		}

		if (ifset) {
			{ //scope for lock
				WorkerEventLocker locker(*this,iftry);
				if (locker.locked) {
					if (!critical) {
						lastStatus = arg_workerEvent->status;
					}
					events.push_back(arg_workerEvent);
				}
			} //unlock worker event
#if ( defined(WIN32) || defined(WIN64) )				
			SetEvent(hNewWorkerEvent);
#else  //UNIX
			pthread_mutex_lock(&mutex_cond_workerEvent);
			bNewWorkerEvent = true;
			pthread_cond_signal(&cond_workerEvent);
			pthread_mutex_unlock(&mutex_cond_workerEvent);
#endif
		}
	}
#else
		throw "OUPUT_MODE_STANDOUT must be using if not using messaging!";
#endif
}

#ifdef USE_MESSAGING
/**
 * Keep trying to start the connection.  Once it is established,
 * setup the message handlers and publishers. (This is synchronized
 * because it can be called from the main thread, and from the
 * onException handler.)
 */
void SimulationMessaging::setupConnection () { //synchronized
	// lockMessaging();
	// unlockMessaging();
}

void SimulationMessaging::cleanup(){
}


/**
 * Suspend the current thread for "duration" milliseconds.
 */
void SimulationMessaging::delay(int duration)
{
#if ( defined(WIN32) || defined(WIN64) )
    Sleep(duration);
#else // UNIX
    sleep(duration/1000);
#endif
}

bool SimulationMessaging::lockMessaging()
{
#if ( defined(WIN32) || defined(WIN64) )
    EnterCriticalSection(&lockForMessaging);
	return true;
#else // UNIX
    if (pthread_mutex_lock(&mutex_messaging)) {
        cout << "Cannot acquire mutex, fatal error." << endl;
        exit(1);
    }
#endif
}

void SimulationMessaging::unlockMessaging()
{
#if ( defined(WIN32) || defined(WIN64) )
    LeaveCriticalSection(&lockForMessaging);
#else // UNIX
    pthread_mutex_unlock(&mutex_workerEvent);
#endif
}


bool SimulationMessaging::lockWorkerEvent(bool bTry)
{	
#if ( defined(WIN32) || defined(WIN64) )
	if (bTry) {
		return TryEnterCriticalSection(&lockForWorkerEvent)!=0;
	}
	try {
		EnterCriticalSection(&lockForWorkerEvent);
	} catch (...) {
		return false;
	}
	return true;
#else // UNIX
	if (bTry) {
		return (pthread_mutex_trylock(&mutex_workerEvent) == 0);
	}
    if (pthread_mutex_lock(&mutex_workerEvent) != 0) {
        return false;
    }
	return true;
#endif
}

void SimulationMessaging::unlockWorkerEvent()
{
#if ( defined(WIN32) || defined(WIN64) )
    LeaveCriticalSection(&lockForWorkerEvent);
#else // UNIX
    pthread_mutex_unlock(&mutex_workerEvent);
#endif
}

char* SimulationMessaging::trim(char* str) {	
	int leftIndex, rightIndex;
	int len = (int)strlen(str);
	for (leftIndex = 0; leftIndex < len; leftIndex ++) { // remove leading spaces
		char c = str[leftIndex];
		if (c != ' ' && c != '\n' && c != '\r') {
			break;
		}
	}
	for (rightIndex = len - 1; rightIndex >= 0; rightIndex --) { // remove trailing spaces and new line and carriage return		
		char c = str[rightIndex];
		if (c != ' ' && c != '\n' && c != '\r') {
			break;
		}
	}

	len = rightIndex - leftIndex + 2;
	if (len <= 0) {
		return 0;
	}

	char* newstr = new char[len];
	memset(newstr, 0, len * sizeof(char));
	strncpy(newstr, str + leftIndex, len - 1);

	return newstr;
}

void SimulationMessaging::keepAlive() {
	if (workerEventOutputMode == WORKEREVENT_OUTPUT_MODE_MESSAGING) {


		CURL* curl = curl_easy_init();
		if(curl) {
			// First set the URL that is about to receive our POST. This URL can
			// just as well be a https:// URL if that is what should receive the
			// data.
			curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:6161/message/workerevent?readTimeout=20000&type=queue");


			// Now specify the POST data
			std::stringstream ss;

			//status
			// msg->setIntProperty(WORKEREVENT_STATUS, JOB_WORKER_ALIVE);
			ss << WORKEREVENT_STATUS << "=" << JOB_WORKER_ALIVE;

			std::string x = ss.str();
			const char* postfields = x.c_str();

			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields); // e.g. postfields="name=daniel&project=curl"
		 
			// Perform the request, res will get the return code
			CURLcode res = curl_easy_perform(curl);
			// Check for errors
			if(res != CURLE_OK)
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		 
			// always cleanup
			curl_easy_cleanup(curl);
		}

		time(&lastSentEventTime);
	}
}

const char* SimulationMessaging::getStatusString(int status) {
	switch (status) {
	case JOB_STARTING:
		return "JOB_STARTING";
		
	case JOB_PROGRESS:
		return "JOB_PROGRESS";
	
	case JOB_DATA:
		return "JOB_DATA";

	case JOB_COMPLETED:
		return "JOB_COMPLETED";

	case JOB_WORKER_ALIVE:
		return "JOB_WORKER_ALIVE";

	case JOB_FAILURE:
		return "JOB_FAILURE";

	default:
		return "Unknown status";
	}
}

SimulationMessaging* SimulationMessaging::create(const char* broker, const char* smqusername, const char* passwd, const char* qname, const char* tname, const char* vcusername, int simKey, int jobIndex, int taskID, int ttl_low, int ttl_high)
{
	if (m_inst != NULL && m_inst->workerEventOutputMode == WORKEREVENT_OUTPUT_MODE_STDOUT) {
		delete m_inst;
		m_inst = NULL;
	}
	if (m_inst == NULL){    
        m_inst = new SimulationMessaging(broker, smqusername, passwd, qname, tname, vcusername, simKey, jobIndex, taskID, ttl_low, ttl_high);
	}

    return(m_inst);
}

void SimulationMessaging::waitUntilFinished() {
	if (workerEventOutputMode == WORKEREVENT_OUTPUT_MODE_STDOUT) {
		return;
	}
#if ( defined(WIN32) || defined(WIN64) )
	DWORD dwWaitResult = WaitForSingleObject(hMessagingThreadEndEvent, INFINITE);
	switch (dwWaitResult) {
	case WAIT_OBJECT_0: 
	case WAIT_TIMEOUT: 	
		break; 

	default:
		cout << "Wait error: " << GetLastError() << endl;
		break;
	}
#else
	cout << "!!!waiting for thread to exit" << endl;
	pthread_join(newWorkerEventThread, NULL);
#endif
}

void* startMessagingThread(void* param);

void SimulationMessaging::start() {
	if (workerEventOutputMode == WORKEREVENT_OUTPUT_MODE_STDOUT) {
		return;
	}
	if (bStarted) {
		return;
	}
	bStarted = true;

#if ( defined(WIN32) || defined(WIN64) )
	HANDLE hThread; 
	DWORD IDThread; 

	char messagingThreadEndEventName[256];
	char newWorkerEventName[256];
	sprintf(messagingThreadEndEventName, "MessagingThreadEndEvent_%ld_%d_%d", m_simKey, m_jobIndex, m_taskID);
	sprintf(newWorkerEventName, "NewWorkerEvent_%ld_%d_%d", m_simKey, m_jobIndex, m_taskID);

	// Create a manual-reset event object. The master thread sets 
	// this to nonsignaled when it writes to the shared buffer.
	hMessagingThreadEndEvent = CreateEvent( 
		NULL,         // no security attributes
		true,         // manual-reset event
		false,         // initial state is nonsignaled
		messagingThreadEndEventName  // object name
	); 

	if (hMessagingThreadEndEvent == NULL) { 
		throw "CreateEvent failed for new worker event";
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		throw "Event for messagingThreadEndEvent already exists";
	}

	hNewWorkerEvent = CreateEvent( 
		NULL,         // no security attributes
		true,         // manual-reset event
		false,         // initial state is nonsignaled
		newWorkerEventName  // object name
	); 

	if (hNewWorkerEvent == NULL) { 
		throw "CreateEvent failed for new worker event";
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		throw "Event for newWorkerEvent already exists";
	}

	// Create multiple threads and an auto-reset event object 
	// for each thread. Each thread sets its event object to 
	// signaled when it is not reading from the shared buffer. 

	hThread = CreateThread(NULL, 0, 
		(LPTHREAD_START_ROUTINE) startMessagingThread, 
		&hNewWorkerEvent,  // pass event handle
		0, &IDThread); 
	if (hThread == NULL) 
	{
		throw "CreateThread failed for messaging thread";
	}
#else // UNIX
	int retv = pthread_create(&newWorkerEventThread, NULL, startMessagingThread, this);
#endif
}


void* startMessagingThread(void* lpParam){	
	SimulationMessaging::getInstVar()->setupConnection();

#if ( defined(WIN32) || defined(WIN64) )
    DWORD dwWaitResult;
    HANDLE hEvent; 
	
    hEvent = *(HANDLE*)lpParam;  // thread's read event

	while (true) {
		dwWaitResult = WaitForSingleObject(hEvent, 5*60*1000);    // 5 min wait

		switch (dwWaitResult) {
		case WAIT_OBJECT_0: 
			{
				WorkerEvent* sentWorkerEvent = SimulationMessaging::getInstVar()->sendStatus();
				SimulationMessaging::getInstVar()->lockWorkerEvent();
				if (sentWorkerEvent != NULL 
					&& SimulationMessaging::getInstVar()->getWorkerEvent() != NULL
					&& sentWorkerEvent->equals(SimulationMessaging::getInstVar()->getWorkerEvent())) {
					ResetEvent(hEvent);
				}
				SimulationMessaging::getInstVar()->unlockWorkerEvent();		
				delete sentWorkerEvent;
			}
			break; 		

			// An error occurred.
		case WAIT_TIMEOUT: 
			SimulationMessaging::getInstVar()->keepAlive();
			break; 

		default:
			std::cout<< "Wait error: " << GetLastError() << endl;
			break;
		}
	}
#else // UNIX
	int waitReturn = 0;
	struct timespec timeout;
	struct timeval now;
	struct timeval start;
	SimulationMessaging* simMessaging = (SimulationMessaging*)lpParam;

	while (true) {
		waitReturn = 1;
		gettimeofday(&start, NULL);
		timeout.tv_sec = start.tv_sec;
		// condition might be signalled before this thread gets blocked
		// so this thread might miss signal and doesn't get wakened
		// if this happens don't want to wait too long to check if there 
		// is new worker event. This is the reason for the loop and 2 sec timeout
		pthread_mutex_lock(&simMessaging->mutex_cond_workerEvent);
		while (timeout.tv_sec - start.tv_sec < 5 * 60) {
			// at this point, there are two possibilities
			// 1. this thread has been awakened, waited < 2 secs, waitReturn is 0
			// 2. this thread has waited full 2 sec timout, waitReturn is non zero
			// in either case, check if there is new worker event.
			if (simMessaging->bNewWorkerEvent) {
				simMessaging->bNewWorkerEvent = false;
				waitReturn = 0;
				break;
			}
			gettimeofday(&now, NULL);
			timeout.tv_sec = now.tv_sec + 2;
			timeout.tv_nsec = now.tv_usec * 1000;
			waitReturn = pthread_cond_timedwait(&simMessaging->cond_workerEvent, &simMessaging->mutex_cond_workerEvent, &timeout);			
		}
		pthread_mutex_unlock(&simMessaging->mutex_cond_workerEvent);
		
		switch (waitReturn) {
			case 0:  { // new event
				simMessaging->sendStatus();
				break; 
			}

			case ETIMEDOUT: // time out
				simMessaging->keepAlive();
				break; 

			default:
				std::cout<< "Wait error: " << waitReturn << endl;
				break;
		}
	}
#endif		
}
#endif
