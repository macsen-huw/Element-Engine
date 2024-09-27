#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "ThreadManager.hpp"
#include "Logger.hpp"
#include "core.hpp"

using namespace elmt;

ThreadManager::~ThreadManager() {
	closeLoopThread(renderThreadData);
	closeLoopThread(physicsThreadData);

	// Make sure main thread has the render context
	if (!hasRenderContext()) {
		seizeRenderContext();
	}
}

int ThreadManager::Setup()
{
	// Only one thread can have the OpenGL context at a time
	glfwMakeContextCurrent(NULL);
	
	startLoopThread(renderThreadData, ThreadManager::renderLoopThread);
	startLoopThread(physicsThreadData, ThreadManager::physicsLoopThread);

	return 0;
}


void ThreadManager::renderLoopThread() {
	Logger::Print("Entering render thread...", LOGCAT::CAT_CORE | LOGCAT::CAT_RENDERING, LOGSEV::SEV_TRACE);

	ThreadManager* tm = core::getThreadManager();
	LoopThreadData& t = tm->renderThreadData;

	while (!t.closing) {
		// Acquire render lock
		int doExit = tm->waitOnLoopThread(t);
		if (doExit) {
			break;
		}


		// Make sure we have the render context
		if (!tm->hasRenderContext() ) {
			tm->seizeRenderContext();
		}

		// Actually render
		core::renderFrame(true);

		

		glfwSwapBuffers(elmt::core::getWindow());

		// Now that rendering is done, unlock the lock so core knows we can go to the next frame
		tm->pauseOnLoopThread(t);

	}

	// Get rid of render context
	if (tm->hasRenderContext()) {
		tm->releaseRenderContext();
	}

	t.closed = true;
	tm->pauseOnLoopThread(t);

	Logger::Print("Exiting render thread...", LOGCAT::CAT_CORE | LOGCAT::CAT_RENDERING, LOGSEV::SEV_TRACE);
	
}

void ThreadManager::physicsLoopThread() {
	Logger::Print("Entering physics thread...", LOGCAT::CAT_CORE | LOGCAT::CAT_RENDERING, LOGSEV::SEV_TRACE);

	ThreadManager* tm = core::getThreadManager();
	LoopThreadData& t = tm->physicsThreadData;

	while (!t.closing) {
		// Acquire physics lock
		int doExit = tm->waitOnLoopThread(t);
		if (doExit){
			break;
		}

		// Actually do physics update
		core::updateFrame(true);


		// Now that physics is done, unlock the lock so core knows we can go to the next frame
		tm->pauseOnLoopThread(t);
	}

	t.closed = true;
	tm->pauseOnLoopThread(t);

	Logger::Print("Exiting physics thread...", LOGCAT::CAT_CORE | LOGCAT::CAT_RENDERING, LOGSEV::SEV_TRACE);

}

int ThreadManager::startLoopThread(LoopThreadData& t, std::function<void()> loopFunction)
{
	t.loopThread = std::thread(loopFunction);
	return 0;
}

int ThreadManager::closeLoopThread(LoopThreadData& t)
{
	t.closing = true;
	continueLoopThread(t);
	waitForLoopThread(t);
	t.loopThread.join();
	return 0;
}


int ThreadManager::waitOnLoopThread(LoopThreadData& t)
{
	t.lock = std::unique_lock(t.mutex);
	t.condition.wait(t.lock, [&t] { return t.started; });
	
	if (t.closed) {
		size_t threadIDNum = getThreadIDNum(std::this_thread::get_id());
		Logger::Print("Loop thread " + std::to_string(threadIDNum) + " started new iteration, but was closed",
			LOGCAT::CAT_CORE, LOGSEV::SEV_WARNING);
		return 1;
	}
	if (t.closing) {
		return 1;
	}

	return 0;
}

int ThreadManager::waitForLoopThread(LoopThreadData& t)
{
	// Don't use t.lock here, that's being used by the loop thread which needs to keep track of it
	// We only need a temporary lock here to wait
	std::unique_lock lk(t.mutex);
	t.condition.wait(lk, [&t] {return t.finished; });

	return 0;
}

int ThreadManager::pauseOnLoopThread(LoopThreadData & t)
{
	t.started = false;
	t.finished = true;
	t.lock.unlock();
	t.condition.notify_one();
	return 0;
}

int ThreadManager::continueLoopThread(LoopThreadData& t)
{
	t.finished = false;
	t.started = true;
	t.condition.notify_one();
	return 0;
}


int ThreadManager::seizeRenderContext() {
	std::thread::id thisThread = std::this_thread::get_id();
	if (thisThread == threadWithRenderContext) {
		size_t threadIDNum = getThreadIDNum(std::this_thread::get_id());
		Logger::Print("Thread " + std::to_string( threadIDNum ) + " attempted to seize render context, but it already had it",
			LOGCAT::CAT_CORE, LOGSEV::SEV_WARNING);
		return 1;

	}
	// Only one thread can have the OpenGL context at a time
	glfwMakeContextCurrent(core::getWindow());
	threadWithRenderContext = thisThread;

	return 0;

}

int ThreadManager::releaseRenderContext() {
	std::thread::id thisThread = std::this_thread::get_id();
	if (thisThread != threadWithRenderContext) {
		size_t threadIDNum = getThreadIDNum(std::this_thread::get_id());
		size_t renderThreadIDNum = getThreadIDNum(threadWithRenderContext);

		Logger::Print("Thread " + std::to_string(threadIDNum) + " attempted to release render context, but it didn't have it ( Context held by thread " +
			std::to_string(renderThreadIDNum) + ")",
			LOGCAT::CAT_CORE, LOGSEV::SEV_WARNING);
		return 1;

	}
	// Only one thread can have the OpenGL context at a time
	glfwMakeContextCurrent(NULL);
	threadWithRenderContext = defaultThreadID;

	return 0;

}

int ThreadManager::addThread(std::thread* newThread) {
	if (frameThreads.size() >= maxAcceptableThreads) {
		Logger::Print("Current number of threads (" + std::to_string(getThreadCount()) + ") is at or exceeds maximum number of threads (" + std::to_string(maxAcceptableThreads) + ")",
			LOGCAT::CAT_CORE, LOGSEV::SEV_WARNING);
	}
	frameThreads.push_back(newThread);

	return 0;
}

int ThreadManager::startFrame() {
	//renderThreadData.started = false;
	//renderThreadData.finished = false;

	//physicsThreadData.started = false;
	//physicsThreadData.finished = false;
	return 0;
}

int ThreadManager::endFrame()
{

	// Ensure all frame-specific threads are finished
	for (std::thread* thread : frameThreads) {
		thread->join();
		delete thread;
	}

	frameThreads.clear();

	// Ensure rendering is done
	waitForLoopThread(renderThreadData);
	
	// Ensure physics is done
	waitForLoopThread(physicsThreadData);

	return 0;
}

