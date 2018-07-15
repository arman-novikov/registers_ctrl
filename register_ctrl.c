#define RULES_MAX_COUNT 10
#define R_FLAG  1 //...001
#define W_FLAG  2 //...010
#define RW_FLAG 3 //...011
#define C_FLAG  4 //...100
#define MAGIC_ADDR 16 //indicates bytes for a struct rules
#define AWAIT_BYTES_FOR_RULES 120
#define AWAIT_BYTES_FOR_CHECK 8

#define __cu8 const __u8

#pragma pack(1)
struct rules {
	struct {
		__u8 cell_addr;
		__u8 fpga_addr;
		__u8 reg_addr;
		__u8 flags;
		__u8 value[8]; // long long as a bytes array
	} rules[RULES_MAX_COUNT];
	__u8  cur_rules_number;
} rules;
#pragma  pack()

static void rules_init(struct rules *p_rules)
{
	memset(p_rules, 0, sizeof(struct rules));
}

static int rules_add_all(struct rules *p_rules, const void *buf)
{
	memcpy(p_rules, buf, sizeof(struct rules)-1);

	p_rules->cur_rules_number = RULES_MAX_COUNT;//size / a_rule_size;
	return 0;
}

static int rules_add_single(struct rules *p_rules, __cu8 cell_addr,
 	__cu8 fpga_addr, __cu8 reg_addr, __cu8 flags, __cu8 *value)
{
	if (p_rules->cur_rules_number >= RULES_MAX_COUNT)
		return -ENOMEM;

	if (!(p_rules->rules[p_rules->cur_rules_number].flags = flags)) // if zeroed flags -> error
		return -EINVAL;

	p_rules->rules[p_rules->cur_rules_number].cell_addr = cell_addr;
	p_rules->rules[p_rules->cur_rules_number].fpga_addr = fpga_addr;
	p_rules->rules[p_rules->cur_rules_number].reg_addr = reg_addr;
	memcpy(p_rules->rules[p_rules->cur_rules_number].value, value, 8);

	p_rules->cur_rules_number++; // the sign of succeessful adding

	return 0;
}

static void print_check(const struct rules *p_rules, __cu8 rule_num,
						__cu8 rw_flag, __cu8 *value)
{
	__u8 i,  cell_addr,  fpga_addr, reg_addr;

	cell_addr = p_rules->rules[rule_num].cell_addr;
	fpga_addr = p_rules->rules[rule_num].fpga_addr;
	reg_addr = p_rules->rules[rule_num].reg_addr;

	printk(KERN_ERR "\nrule #%i was triggered: ", rule_num);
	switch (p_rules->rules[rule_num].flags) {
	case R_FLAG:
		printk(KERN_ERR "\t(read)");
		break;
	case W_FLAG:
		printk(KERN_ERR "\t(write)");
		break;
	case RW_FLAG:
		printk(KERN_ERR "\t(read + write)");
		break;
	case C_FLAG + R_FLAG:
		printk(KERN_ERR "\t(valued read)");
		break;
	case C_FLAG + W_FLAG:
		printk(KERN_ERR "\t(valued write)");
		break;
	case C_FLAG + R_FLAG + W_FLAG:
		printk(KERN_ERR "\t(valued read + write)");
		break;
	default:
		break;
	}

	(rw_flag == R_FLAG)? printk(KERN_ERR " read: "): printk(KERN_ERR " write: ");
	printk(KERN_ERR "\tcaddr = %i, faddr = %i, raddr = %i, value = ",
	 	cell_addr, fpga_addr, reg_addr);
	for (i = 0; i < size; i++)
		printk(KERN_ERR "%x", value[i]);

	printk(KERN_ERR "\n\n");
}

static int check(const struct rules *p_rules, __cu8 cell_addr, __cu8 fpga_addr,
 	__cu8 reg_addr, __cu8 rw_flag, __cu8 *value, const size_t size)
{
	unsigned char i;
	unsigned char exit_flag;
	unsigned char rule_flag;

	exit_flag = 1;
	for (i = 0; i < RULES_MAX_COUNT && exit_flag != 0; i++) {
		rule_flag = p_rules->rules[i].flags;

		if (rule_flag > C_FLAG)
			rule_flag -= C_FLAG;

		if (p_rules->rules[i].cell_addr == cell_addr &&
		    p_rules->rules[i].fpga_addr == fpga_addr &&
		    p_rules->rules[i].reg_addr == reg_addr &&
		    (rule_flag == RW_FLAG || rule_flag == rw_flag) )
				exit_flag = 0;
	}

	if (exit_flag)
		return -EAGAIN;

	if (p_rules->rules[i].flags > C_FLAG) {
		int res;
		res = memcmp(p_rules->rules[i].value, value, size);
		if (res == 0)
			print_check(p_rules, i, rw_flag, value);
		return 0;
	}

	print_check(p_rules, i, rw_flag, value);

	return 0;
}

#undef RULES_MAX_COUNT
#undef R_FLAG
#undef W_FLAG
#undef RW_FLAG
#undef C_FLAG
#undef MAGIC_ADDR 16
#undef AWAIT_BYTES_FOR_RULES
#undef AWAIT_BYTES_FOR_CHECK

#undef __cu8 

//endofadd