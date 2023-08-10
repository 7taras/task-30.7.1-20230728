#include "RequestHandler.h"

RequestHandler::RequestHandler() 
{
	m_tpool.start();
}

RequestHandler::~RequestHandler() 
{
	m_tpool.stop();
}

void RequestHandler::pushRequest(FuncType f, int* array, int begin, int end)
{
	m_tpool.push_task(f, array, begin, end);
}