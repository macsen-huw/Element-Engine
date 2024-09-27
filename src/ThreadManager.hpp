#pragma once

#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>

/*
class core {
public:
	static bool core::renderFrame(bool isThreaded);
};

*/
namespace elmt {

	/*
	Data used for managing things like physics and rendering, which run continously on dedicated threads
	Use the associated LoopThread functions to work with them
	*/
	struct LoopThreadData {
		
		std::mutex mutex; // Used by lock to wait
		std::unique_lock<std::mutex> lock; // Used by condition to wait
		std::condition_variable condition; // Used for waiting

		bool started = false; // Loop thread waits until this is set
		bool finished = false;// Main thread thread waits until this is set
		std::thread loopThread; // The actual thread itself

		bool closing = false; // Set this to break the loop of the loop thread
		bool closed = false; // Set by loop thread when it is about to exit
	};

	class ThreadManager
	{
		// Attributes
	private:
		// All single-frame threads that must be completed before endFrame will exit
		std::vector<std::thread*> frameThreads;

		/*
		The maximum number of threads that the engine will accept running simultaneously (not including main thread)
		If this number is reached, new threads can still be added but warnings will be given
		*/
		unsigned int maxAcceptableThreads = 8;

		friend class core;

		LoopThreadData renderThreadData;
		LoopThreadData physicsThreadData;

		
		// Thread which currently has render context
		std::thread::id threadWithRenderContext;
		// Used if no thread has the render context
		std::thread::id defaultThreadID;


		// Methods
	public:
		ThreadManager() {};
		~ThreadManager();

		int Setup();

		size_t getThreadCount() { return frameThreads.size(); }

		/*
			Add a thread that is performing some engine task to the internal thread system
			endFrame will wait for all threads to finish before progressing to the next frame
			*/
		int addThread(std::thread* newThread);

		/*
		* Get the thread ID in size_t form
		From https://stackoverflow.com/questions/7432100/how-to-get-integer-thread-id-in-c11
		*/
		size_t getThreadIDNum(std::thread::id threadID) { return std::hash<std::thread::id>{}(threadID); }

		/*
		If multithreading is enabled, this provides a continuous loop for rendering to run on
		It's static so it can be used as a thread callback
		*/
		static void renderLoopThread();
		/*
		If multithreading is enabled, this provides a continuous loop for physics to run on
		It's static so it can be used as a thread callback
		*/
		static void physicsLoopThread();


		// LOOP THREAD METHODS
		// (Main Thread) Start the actual thread of the loop thread
		int startLoopThread(LoopThreadData& t, std::function<void()> loopFunction );
		// (Main Thread) Tell a loop thread to exit it's loop and join with main thread
		int closeLoopThread(LoopThreadData& t);

		// (Loop Thread) Wait until this loop thread is next continued. If the loop thread is closing or closed, 1 is returned
		inline int waitOnLoopThread(LoopThreadData& t);
		// (Main Thread) Wait until this loop thread finishes it's current iteration
		int waitForLoopThread(LoopThreadData& t);

		// (Loop Thread) Signal the loop thread is done with it's iteration
		inline int pauseOnLoopThread(LoopThreadData& t);
		// (Main Thread) Continue this loop thread, allowing it to start a new iteration
		int continueLoopThread(LoopThreadData& t);
		

		int startFrame();
		int endFrame();

		/*
		Give the OpenGL context to the caller thread
		*/
		int seizeRenderContext();
		/*
		If this thread has the render context, release it
		*/
		int releaseRenderContext();

		// Check if the caller thread has the render context
		bool hasRenderContext() {
			if (std::this_thread::get_id() == threadWithRenderContext) { return true; }
			else { return false; }
		}

		// Check if the render thread has the render context
		bool renderThreadHasContext() {
			if (renderThreadData.loopThread.get_id() == threadWithRenderContext) { return true; }
			else { return false; }
		};
	};
}

