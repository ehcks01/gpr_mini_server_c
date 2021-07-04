#ifndef GPR_TIMING__h
#define GPR_TIMING__h

// Only add new flags, don't change or remove:
#define VAR_IS_VISIBLE 1		   ///< The variable is visible.
#define VAR_IS_READONLY 2		   ///< The variable is read only.
#define VAR_HAS_AUTO 4			   ///< \deprecated Not in use.
#define VAR_IS_ENUM 8			   ///< The variable is of enum type.
#define VAR_IS_LIVEREAD 16		   ///< The variable is live read.
#define VAR_IS_SAMPLED_LIVEREAD 32 ///< The variable is a sampled live read.
#define VAR_IS_LIVEAUTO 64		   ///< The variable is a live auto.
#define VAR_IS_AUTO 128			   ///< The variable is currently an auto variable. The auto value funtion is used to set its value.
#define VAR_IS_APP_WRITABLE 256	   ///< The application can set new value, but the variable system interface can not alter the value.
#define VAR_IS_ACTION 512		   ///< The variable is an action trigger.
#define VAR_IS_REGISTER_READ 1024  ///< The variable is connected to a read only register.
#define VAR_IS_INT 2048			   ///< The variable is of type integer.
#define VAR_IS_FLOAT 4096		   ///< The variable is of type floating point.
#define VAR_IS_INT_ARRAY 8192	   ///< Not implemented.

#define SUCCESS 0
#define FAILED 1

struct DelayElement
{
	int Flag;
	int Value;
};

void MeasureWait();
void VarSetIntValue(struct DelayElement delayElement, int value);
int GetValue(struct DelayElement delayElement);
int GPR_FindLevel(struct DelayElement delayElement, int delayElementMax, int level, int start);
int GPR_FindEdge(struct DelayElement delayElement, int delayElementMax, int edgeDirection, int start);
float GPR_TimingMeasurementValueLiveRead(int deviceNumber);

#endif