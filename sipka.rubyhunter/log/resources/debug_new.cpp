#undef new
#include <new>
#include <framework/threading/Thread.h>

class AllocationData {
public:
	static AllocationData ad_start;
	static AllocationData ad_end;

	size_t count;
	const char* filename;
	int line;
	rhfw::Thread::Id tid;
	AllocationData* prev;
	AllocationData* next;
	bool isarray;
};

AllocationData AllocationData::ad_start { 0, nullptr, 0, 0, nullptr, &ad_end, false };
AllocationData AllocationData::ad_end { 0, nullptr, 0, 0, &ad_start, nullptr, false };

#include <framework/threading/Mutex.h>

static rhfw::Mutex& getMutex() {
	struct PersistentMutex {
		rhfw::Mutex* mtx = (rhfw::Mutex*) malloc(sizeof(rhfw::Mutex));
		PersistentMutex() {
			::new (mtx) rhfw::Mutex(rhfw::Mutex::auto_init { });
		}
	};
	static PersistentMutex alloc_tracker_mtx;
	return *alloc_tracker_mtx.mtx;
}

//namespace std { class nothrow_t {}; } // namespace std

template<bool isarray> static void* do_allocation(size_t count, const char* filename, int line) {
	void* ptr = malloc(count + sizeof(AllocationData));
	if (ptr == nullptr) {
		return nullptr;
	}

	getMutex().lock();
@if this.logDebugNew:
	LOGV() << "New" << (isarray ? "[]" : "") << " at: " << filename << ":" << line << ", size: " << count << ", ptr: " << ptr << ", tid: "
			<< (size_t) rhfw::Thread::getCurrentId();
@
	*((AllocationData*)ptr)= {count, filename, line, rhfw::Thread::getCurrentId(), AllocationData::ad_end.prev, &AllocationData::ad_end, isarray};
	AllocationData::ad_end.prev->next = (AllocationData*) ptr;
	AllocationData::ad_end.prev = (AllocationData*) ptr;
	getMutex().unlock();
	return ((char*) ptr) + sizeof(AllocationData);
}

template<bool isarray> static void do_freeing(void* ptr) {
	if (ptr != nullptr) {
		AllocationData* d = (AllocationData*) (((char*) ptr) - sizeof(AllocationData));

		getMutex().lock();
		bool found = false;
		for (AllocationData* i = &AllocationData::ad_start; i != &AllocationData::ad_end; i = i->next) {
			if (i == d) {
				found = true;
				break;
			}
		}

		WARN(!found) << "Trying to delete not allocated pointer: " << d;
@if this.logDebugNew:
		LOGV() << "Delete" << (isarray ? "[]" : "") << ": " << d->filename << ":" << d->line << ", size: " << d->count << ", ptr: " << d
				<< ", tid: " << (size_t) rhfw::Thread::getCurrentId();
@
		ASSERT(found) << "Deleting not allocated memory region. See previous log messages for more info."
@if !this.logDebugNew:
		<< "Delete" << (isarray ? "[]" : "") << ": " << d->filename << ":" << d->line << ", size: " << d->count << ", ptr: " << d
				<< ", tid: " << (size_t) rhfw::Thread::getCurrentId();
@
		;
		ASSERT(d->isarray == isarray)
				<< (isarray ? "trying to delete[] ptr that was allocated with new" : "trying to delete ptr that was allocated with new[]");
		if (d->prev != nullptr) {
			d->prev->next = d->next;
		}
		if (d->next != nullptr) {
			d->next->prev = d->prev;
		}
		getMutex().unlock();
		free(d);
	} else {
@if this.logDebugNew:
		LOGV() << "Delete" << (isarray ? "[]" : "") << ": ptr: nullptr, tid: " << (size_t) rhfw::Thread::getCurrentId();
@
	}
}

void* operator new(size_t count, const char* filename, int line)  {
	return do_allocation<false>(count, filename, line);
}

void* operator new[](size_t count, const char* filename, int line)  {
	return do_allocation<true>(count, filename, line);
}

void* operator new(size_t count) {
	return operator new(count, "-UNKNOWN_LOCATION-", -1);
}
void* operator new[](size_t count) {
	return operator new[](count, "-UNKNOWN_LOCATION-", -1);
}

void operator delete(void* ptr) {
	do_freeing<false>(ptr);
}

void operator delete[](void* ptr) {
	do_freeing<true>(ptr);
}

// against visual studio warning C4291
void operator delete[](void* ptr, const char* file, int line)  {
	do_freeing<true>(ptr);
}

// against visual studio warning C4291
void operator delete(void* ptr, const char* file, int line)  {
	do_freeing<false>(ptr);
}

void* operator new(size_t count, const std::nothrow_t&)  {
	return operator new(count, "-UNKNOWN_LOCATION-std::nothrow_t", -1);
}
void* operator new[](size_t count, const std::nothrow_t&)  {
	return operator new[](count, "-UNKNOWN_LOCATION- std::nothrow_t", -1);
}
void operator delete(void* ptr, const std::nothrow_t&)  {
	operator delete(ptr);
}
void operator delete[](void* ptr, const std::nothrow_t&)  {
	operator delete[](ptr);
}

namespace rhfw {
void do_memory_leak_logging() {
	getMutex().lock();
	LOGD() << "Not deallocated memory regions:";
	for (AllocationData* i = AllocationData::ad_start.next; i != &AllocationData::ad_end; i = i->next) {
		LOGD() << i->filename << ":" << i->line << " ptr: " << i << ", size: " << i->count << ", thread: " << (size_t) i->tid
				<< (i->isarray ? ", [ARRAY]" : "");
	}
	LOGD() << "No more entries.";
	getMutex().unlock();
}
void start_memory_leak_logging() {
	getMutex().lock();
	LOGD() << "Start memory tracking...";
	AllocationData* n = AllocationData::ad_start.next;
	AllocationData* p = AllocationData::ad_end.prev;
	n->prev = nullptr;
	p->next = nullptr;
	AllocationData::ad_end.prev = &AllocationData::ad_start;
	AllocationData::ad_start.next = &AllocationData::ad_end;
	getMutex().unlock();
}
} // namespace rhfw
