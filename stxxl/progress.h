#ifndef PROGRESS_H
#define PROGRESS_H

#include <iostream>

inline void end_progress() {
	std::cout << std::endl;
}

inline void set_progress(const char * label, size_t steps, size_t total) {
	std::cout << "\r\e[K" << label << " " << steps << "/" << total << std::flush;
}

#endif // PROGRESS_H
