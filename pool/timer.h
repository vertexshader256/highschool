class timerclass
{
public:
	timerclass();
	int ShowFps(void);
	unsigned long begintime;
	float scalar;
	float timetaken;
	float endtime;
private:
	__int64      frequency;// Timer Frequency
	float         resolution;// Timer Resolution
	unsigned long mm_timer_start;// Multimedia Timer Start Value
	unsigned long mm_timer_elapsed;// Multimedia Timer Elapsed Time
	bool			performance_timer;// Using The Performance Timer?
	__int64       performance_timer_start;// Performance Timer Start Value
	__int64       performance_timer_elapsed;// Performance Timer Elapsed Time
};
