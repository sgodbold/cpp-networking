#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#define BOOST_THREAD_VERSION 4

#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

#include <string>

using namespace boost;

using std::string;
using std::to_string;

int main()
{
    future<int> f1 = async([]() -> int { return int(123); });

    future<string> f2 = f1.then([](future<int> f) {
            int n = f.get();
            return to_string(n);
    });
}

