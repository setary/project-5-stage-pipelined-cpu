.PHONY: all
all: testAssn4V1 testAssn4V2 testAssn4V3

testAssn4V1: testAssn4.cc Memory.h Control.h RegisterFile.h ALU.h ALUControl.h Miscellaneous.h PipelinedCPU.h
	g++ -o $@ -std=c++11 $<

testAssn4V2: testAssn4.cc Memory.h Control.h RegisterFile.h ALU.h ALUControl.h Miscellaneous.h PipelinedCPU.h
	g++ -o $@ -std=c++11 -DENABLE_DATA_FORWARDING $<

testAssn4V3: testAssn4.cc Memory.h Control.h RegisterFile.h ALU.h ALUControl.h Miscellaneous.h PipelinedCPU.h
	g++ -o $@ -std=c++11 -DENABLE_DATA_FORWARDING -DENABLE_HAZARD_DETECTION $<

.PHONY: clean
clean:
	rm -f testAssn4V1 testAssn4V2 testAssn4V3

