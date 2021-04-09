cdef extern void initUS100()
cdef extern void freeUS100()
cdef extern void TXon()
cdef extern void TXoff()
cdef extern int readRX()
cdef extern initTimer()
cdef extern freeTimer()
cdef extern unsigned long long getSystemTimerCounter()

class Ultra:
    #constructor
    def __init__(self):
        initUS100()
        
    #deconstructor
    def __del__(self):
        freeUS100()
    
    def TX_on(self):
        TXon()
    
    def TX_off(self):
        TXoff()
        
    def read_RX(self):
        return readRX()

class Clock:
    #constructor
    def __init__(self):
        initTimer()
    
    #deconstructor    
    def __del__(self):
        freeTimer()
        
    def getTimer(self):
        return getSystemTimerCounter()
            
        
