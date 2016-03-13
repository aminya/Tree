#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>

template<typename Type>
struct TypeName
{
   // @todo constexpr
   static const char* Resolve()
   {
      return typeid(Type).name();
   }
};

template<>
struct TypeName<std::chrono::nanoseconds>
{
   // @todo UPGRADE TO C++11: constexpr
   static const char* Resolve()
   {
      return "nanoseconds";
   }
};

template<>
struct TypeName<std::chrono::microseconds>
{
   // @todo UPGRADE TO C++11: constexpr
   static const char* Resolve()
   {
      return "microseconds";
   }
};

template<>
struct TypeName<std::chrono::milliseconds>
{
   // @todo UPGRADE TO C++11: constexpr
   static const char* Resolve()
   {
      return "milliseconds";
   }
};

template<>
struct TypeName<std::chrono::seconds>
{
   // @todo UPGRADE TO C++11: constexpr
   static const char* Resolve()
   {
      return "seconds";
   }
};

template<>
struct TypeName<std::chrono::minutes>
{
   // @todo UPGRADE TO C++11: constexpr
   static const char* Resolve()
   {
      return "minutes";
   }
};

template<>
struct TypeName<std::chrono::hours>
{
   // @todo UPGRADE TO C++11: constexpr
   static const char* Resolve()
   {
      return "hours";
   }
};

/**
* @brief The Stopwatch class will wrap the function to be timed in a timing block, and then, based
* on which constructor was called, pass the resulting timing information to either std::cout or a
* user-defined function upon destruction of the class.
*
* @tparam ChronoType               One of the following std::chrono time representations:
*                                     @li std::chrono::nanoseconds
*                                     @li std::chrono::microseconds
*                                     @li std::chrono::milliseconds
*                                     @li std::chrono::seconds
*                                     @li std::chrono::minutes
*                                     @li std::chrono::hours
*/
template<typename ChronoType>
class Stopwatch
{
public:
   using LoggingFunction = std::function<void(std::uint64_t, const std::string&)>;

   /**
   * @brief This Stopwatch constructor executes and times the code encapsulated within the
   * std::function object.
   *
   * Once the std::function has completed execution, the timing results and corresponding units
   * will be passed to the specified callback function.
   *
   * @param[in] functionToTime  std::function encapsulating the code to be timed.
   * @param[in] logger          Callback to handle the timing result.
   */
   Stopwatch(
      const std::function<void()>& functionToTime,
      const LoggingFunction&& logger)
   {
      Time(functionToTime);

      if (logger)
      {
         logger(m_elapsed.count(), TypeName<ChronoType>::Resolve());
      }
   }

   /**
   * @brief This Stopwatch constructor will execute and time the code encapsulated within the
   * std::function object.
   *
   * Once the targeted code has completed execution, the timing results will be written out to
   * std::cout, along with the specified message. Specifically, the message will be written out 
   * first, followed immediately by the elapsed time and the corresponding units.
   *
   * @param[in] functionToTime  std::function encapsulating the code to be timed.
   * @param[in] message         Output message.
   */
   Stopwatch(
      const std::function<void()>& functionToTime,
      const char* const message)
   {
      Time(functionToTime);

      std::cout << message << m_elapsed.count() << " "
         << TypeName<ChronoType>::Resolve() << "." << std::endl;
   }

   /**
   * @brief This Stopwatch constructor will time the code encapsulated in the std::function object
   * and then same the result to a member variable.
   *
   * In order to retrieve the elapsed time, call GetElapsedTime().
   */
   Stopwatch(const std::function<void()>& functionToTime)
   {
      Time(functionToTime);
   }

   /**
   * @returns The elapsed time in units of type ChronoType.
   */
   std::uint64_t GetElapsedTime()
   {
      return m_elapsed.count();
   }

private:
   void Time(const std::function<void()>& functionToTime)
   {
      const auto start = std::chrono::high_resolution_clock::now();

      functionToTime();

      const auto end = std::chrono::high_resolution_clock::now();
      m_elapsed = std::chrono::duration_cast<ChronoType>(end - start);
   }

   ChronoType m_elapsed;
};

#define TIME_IN_NANOSECONDS(code, message)   \
   Stopwatch<std::chrono::nanoseconds>(      \
      [&] { code; },                         \
      message);

#define TIME_IN_MICROSECONDS(code, message)  \
   Stopwatch<std::chrono::microseconds>(     \
      [&] { code; },                         \
      message);

#define TIME_IN_MILLISECONDS(code, message)  \
   Stopwatch<std::chrono::milliseconds>(     \
      [&] { code; },                         \
      message);

#define TIME_IN_SECONDS(code, message)       \
   Stopwatch<std::chrono::seconds>(          \
      [&] { code; },                         \
      message);

#define TIME_IN_MINUTES(code, message)       \
   Stopwatch<std::chrono::minutes>(          \
      [&] { code; },                         \
      message);

#define TIME_IN_HOURS(code, message)         \
   Stopwatch<std::chrono::hours>(            \
      [&] { code; },                         \
      message);

#endif // STOPWATCH_H
