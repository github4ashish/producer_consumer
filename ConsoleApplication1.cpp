// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <iostream>
#include <mutex>
#include <stack>
#include <vector>
#include <thread>

using namespace std;

class ResoucesMgr {

public:
	void push(int ele)
	{
		lock_guard<mutex> lk(m_ResMtx);
		m_res.push(ele);		
	}
	void pop()
	{
		unique_lock<mutex> lk(m_ResMtx);
		m_cond.wait(lk, [this]{return !m_res.empty(); });

		m_res.pop();
	}
	int top()
	{
		unique_lock<mutex> lk(m_ResMtx);
		m_cond.wait(lk, [this] {return !m_res.empty(); });

		return m_res.top();
	}
private:
	stack<int> m_res;
	mutex m_ResMtx;
	condition_variable m_cond;
};

class Producer {
	ResoucesMgr &m_resMgr;
public:
	Producer(ResoucesMgr& resMgr) : m_resMgr(resMgr) {}

	void operator()(int val)
	{
		for (int i = 0; i < 20; ++i)
		{
			m_resMgr.push(val * i);
		}
	}
};

class Consunmer {
	ResoucesMgr &m_resMgr;
public:
	Consunmer(ResoucesMgr& resMgr) : m_resMgr(resMgr) {}

	void operator()()
	{
		for (int i = 0; i < 20; ++i)
		{
			int ele = m_resMgr.top();
			cout << "[" << this_thread::get_id() << "]: " << ele << endl;
			m_resMgr.pop();
		}
	}
};

class Singleton {
public:
	static Singleton* getInstance() {

		if (!m_pInstance)
		{
			lock_guard<mutex> lk(m_Instancemtx);
			if (!m_pInstance)
			{
				m_pInstance = new Singleton;
			}
		}

		return m_pInstance;
	}
private:
	Singleton() {}
	Singleton(Singleton const& robj) = delete;
	static Singleton* m_pInstance;
	static mutex m_Instancemtx;
};

int main()
{
	vector<thread> pool;
	ResoucesMgr resMgr;

	int value = 10;
	for (int i = 0; i < 10; ++i)
	{
		pool.push_back(thread(Consunmer(resMgr)));
	}
	for (int i = 1; i <= 10; ++i)
	{
		pool.push_back(thread(Producer(resMgr), value * i));
	}

	for (int i = 0; i < pool.size(); ++i)
	{
		pool[i].join();
	}
}