#ifndef ZEITKATZE_HPP
#define ZEITKATZE_HPP 
#include <atomic> // atomic<T>
#include <chrono> // chrono::duration, chrono::duration_cast
#include <cmath> // pow(), floor()
#include <iostream> // ostream()
#include <sstream>
#include <vector> // vector<T>
#include <string> // string
#include <iomanip> // setw(), setfill()

//-----------------------------------------------------------------------------
// External variable declarations
extern bool color_enabled;
extern std::atomic<bool> interrupted;
extern void interrupt(int);

//-----------------------------------------------------------------------------
// Types/enums/operators
typedef std::vector<std::string> CatVector;
typedef std::vector<std::string>::size_type CatIndex;
using std::chrono::steady_clock;
enum class Color {
	Normal,
	Cat,
	Cat_hold,
	Total,
	Running,
	Running_lap,
	Split,
	Split_lap
};

extern std::ostream& operator<<(std::ostream& oss, Color c); 


//-----------------------------------------------------------------------------
// Class(es) 
class Zeitkatze {
	public:
		Zeitkatze() :
			split_printed_(false),
			start_(steady_clock::now()),
			last_lap_(start_),
			last_line_len_(0)
				{ }
		void print_split_time() { print_time(some_cat_index(), Color::Split); }
		void print_end_time() { print_time(kCats.size() - 1, Color::Total); }
		void print_time(const CatIndex cat_index, const Color color); 
		void print_current_time();
		double elapsed();
		std::string format_seconds(double seconds, unsigned precision = 2);
		CatIndex some_cat_index();
		void reset_laps();

	private:
		bool split_printed_, had_lap_;
		steady_clock::time_point start_, last_lap_;
		unsigned last_line_len_;
        const CatVector kCats = { "=(^.^)=", "=(o.o)=", "=(^.^)\"", "=(x.x)=",
		"=(o.o)m", " (o,o) ", "=(0.0)=", "=(@.@)=", "=(*.*)=", "=(-.-)=", "=(v.v)=", "=(o.O)=",
		"=[˙.˙]=", "=(~.~)=", "=(ˇ.ˇ)=", "=(=.=)=" };


};


#endif /* ZEITKATZE_HPP */
