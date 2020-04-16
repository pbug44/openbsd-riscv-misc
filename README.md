# openbsd-riscv-misc
A miscellaneous share where I keep some external programs

* bt.sh is a script that converts a backtrace from DDB to symbolic names with
	help of an objdump on the kernel.  This is needed because we don't
	have an ELF header and symbols in the BBL loaded kernel.  At least
	that was the result of me dumping memory and not finding any ELF
	magic.

* riscv-opcodes.c	makes the struct riscv64/db_disasm.c in a pretty
			machine way.
