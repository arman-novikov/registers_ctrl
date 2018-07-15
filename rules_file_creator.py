#from copy import deepcopy

# possible flags:
R_FLAG   = 1 
W_FLAG   = 2 
RW_FLAG  = 3 
C_FLAG   = 4
CR_FLAG  = 5
CW_FLAG  = 6
CRW_FLAG = 7
# end of possible flags

MAX_BYTES_COUNT_IN_VALUE = 8
MAX_RULES_COUNT = 10

class Rule:
	def __init__(self, caddr, faddr, raddr, flags, val):
		self.cell_addr = caddr
		self.fpga_addr = faddr
		self.reg_addr = raddr
		self.flags = flags

		self.value = [] # thoroughful control
		i = 0 
		for char in val:
			if i > MAX_BYTES_COUNT_IN_VALUE:
				break
			self.value.append(char) #explicit assignment
			i += 1

class Rules_Manager:
	def __init__(self):
		self.rules_cur_num = 0
		self.rules = []

	def add(self, Rule):
		if self.rules_cur_num > MAX_RULES_COUNT:
			return -12

		self.rules.append(Rule)
		self.rules_cur_num += 1

	def pop(self):
		self.rules = self.rules[:self.rules_cur_num - 1]
		self.rules_cur_num -= 1

	def erase(self):
		self.rules[:] = []
		self.rules_cur_num = 0

	def write(self, fname):
		fout = open(fname, "w+b")
		bytes_for_write = []
		
		for r in self.rules:
			bytes_for_write += list(map(lambda x: x, 
				[r.cell_addr, r.fpga_addr, r.reg_addr, r.flags] + r.value))

		fout.write(bytearray(bytes_for_write))
		fout.close()

def test():
	RM = Rules_Manager()

	i = 0
	while i < MAX_RULES_COUNT:
		rule = Rule(0x10, 0x10, 0x0, CRW_FLAG, [0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, ])
		RM.add(rule)
		i += 1

	RM.write("ft")

test()