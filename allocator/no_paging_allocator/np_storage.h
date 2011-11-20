#pragma once

#include <new>
#include <list>
#include <algorithm>
#include <iostream> //for dump()
template<typename T>
class np_storage
{
	enum ss_defaults
	{
		init_size = 0xfffff
	};
public:
	np_storage() :
	  m_size(
		  sizeof(link) > sizeof(T) ?
		  init_size * sizeof(link) :
	  init_size * sizeof(T)),
		  m_step(sizeof(link) > sizeof(T) ? sizeof(link) : sizeof(T))
	  {
		  m_mem_list.push_back(new char[m_size]);
		  link *l;
		  head_ = l = reinterpret_cast<link *>(*m_mem_list.begin());
		  for (int i = 1; i < init_size; ++i)
		  {
			  l->next_ = reinterpret_cast<link *>(*m_mem_list.begin()
				  + i * m_step);
			  l = l->next_;
		  }
		  l->next_ = 0;
	  }

	  T* allocate()
	  {
		  link *l = head_;
		  if (!l)
		  {
			  throw std::exception("Out of memory");
		  }
		  head_ = head_->next_;
		  return reinterpret_cast<T *>(l);
	  }
	  void* allocate(
		  size_t n)
	  {
		  link *l = head_, *conn, *ret;
		  if (n % m_step)
			  n = m_step * (n / m_step + 1);
		  while (1)
		  {
			  if (verify_memory_contiguous(l->next_, n / m_step))
			  {
				  conn = ret = l->next_;
				  for (unsigned int i = 0; i < n / m_step; ++i)
					  conn = conn->next_;
				  l->next_ = conn;
				  return ret;
			  }
			  if (l->next_)
				  l = l->next_;
			  else
			  {
				  throw std::exception("Out of memory");
			  }
		  }
	  }
	  void deallocate(
		  void *p,
		  size_t n)
	  {
		  if (n <= m_step)
		  {
			  link *l = head_;
			  head_ = reinterpret_cast<link*>(p);
			  head_->next_ = l;
		  }
		  else
		  {
			  link *l = head_, *conn;
			  head_ = reinterpret_cast<link*>(p);
			  conn = head_;
			  for (unsigned int i = 0; i < n / m_step; ++i)
			  {
				  conn->next_ = reinterpret_cast<link*>(p) + i;
				  conn = conn->next_;
			  }
			  conn->next_ = l;
		  }
	  }
	  void dump()
	  {
		  using namespace std;
		  link *l = head_;
		  int links = 0;
		  size_t free_space = 0;
		  while (l->next_)
		  {
			  free_space += m_step;
			  ++links;
			  l = l->next_;
		  }
		  cout << "Free space: " << links * m_step << ", links: " << links
			  << endl;
	  }
	  ~np_storage()
	  {
		  //std::for_each(m_mem_list.begin(), m_mem_list.end(), killer());
	  }

private:
	struct link
	{
		link *next_;
	};
	
	bool verify_memory_contiguous(
		link *l,
		int n)
	{
		if (!l)
			return false;
		for (int i = 0; i < n; ++i)
		{
			if (l->next_)
			{
				if (reinterpret_cast<char *>(l->next_)
					- reinterpret_cast<char *>(l) == m_step)
				{
					l = l->next_;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		return true;
	}
	struct killer
	{
		void
			operator()(
			void *p)
		{
			//delete[] p;
			// this is "no delete" storage
			// the whole chunk will be released when allocating singleton dies
		}
	};
	size_t m_size;
	size_t m_step;
	std::list<char *> m_mem_list;
	link *head_;
};
