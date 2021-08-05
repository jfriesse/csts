.PHONY: all all-flatiron cmap-tests confdb-tests cpg-tests utils sam-tests clean

CFLAGS += -Wall -Wshadow -Wp,-D_FORTIFY_SOURCE=2 -g -O
ALL-VERSION-INDEPENDENT = cpg-tests utils

all: $(ALL-VERSION-INDEPENDENT) cmap-tests sam-tests

all-flatiron: $(ALL-VERSION-INDEPENDENT) confdb-tests

CMAP-TESTS-BIN = cmap-getset cmap-incdec cmap-iter cmap-memleak
cmap-tests: $(CMAP-TESTS-BIN)

CONFDB-TESTS-BIN = confdb-getset confdb-incdec confdb-iter confdb-memleak confdb-track-and-change
confdb-tests: $(CONFDB-TESTS-BIN)

CPG-TESTS-BIN = cpg-cli-client cpg-confchg cpg-init-load cpg-load testcpg free-outq-items-on-exit
cpg-tests: $(CPG-TESTS-BIN)

UTILS-BIN = file-change-bytes stdin-to-usock usock-to-stdout
utils: $(UTILS-BIN)

SAM-TESTS-BIN = sam-test1 sam-test2 sam-test3 sam-test4 sam-test5 sam-test6 sam-test7 \
    sam-test8 sam-test9
sam-tests: $(SAM-TESTS-BIN)

cmap-getset: cmap-getset.c
	$(CC) $(CFLAGS) $< -lcmap -o $@

cmap-incdec: cmap-incdec.c
	$(CC) $(CFLAGS) $< -lcmap -o $@

cmap-iter: cmap-iter.c
	$(CC) $(CFLAGS) $< -lcmap -o $@

cmap-memleak: cmap-memleak.c
	$(CC) $(CFLAGS) $< -lcmap -o $@

confdb-getset: confdb-getset.c
	$(CC) $(CFLAGS) $< -lconfdb -o $@

confdb-incdec: confdb-incdec.c
	$(CC) $(CFLAGS) $< -lconfdb -o $@

confdb-iter: confdb-iter.c
	$(CC) $(CFLAGS) $< -lconfdb -o $@

confdb-memleak: confdb-memleak.c
	$(CC) $(CFLAGS) $< -lconfdb -o $@

confdb-track-and-change: confdb-track-and-change.c
	$(CC) $(CFLAGS) $< -lconfdb -o $@

cpg-cli-client: cpg-cli-client.c
	$(CC) $(CFLAGS) $< -lcpg -o $@

cpg-confchg: cpg-confchg.c
	$(CC) $(CFLAGS) $< -lcpg -o $@

cpg-init-load: cpg-init-load.c
	$(CC) $(CFLAGS) $< -lcpg -o $@

cpg-load: cpg-load.c
	$(CC) $(CFLAGS) $< -lcpg -o $@

testcpg: testcpg.c
	$(CC) $(CFLAGS) $< -lcpg -o $@

file-change-bytes: file-change-bytes.c
	$(CC) $(CFLAGS) $< -o $@

free-outq-items-on-exit: free-outq-items-on-exit.c
	$(CC) $(CFLAGS) $< -lcpg -o $@

sam-test1: sam-test1.c
	$(CC) $(CFLAGS) $< -lsam -o $@

sam-test2: sam-test2.c
	$(CC) $(CFLAGS) $< -lsam -o $@

sam-test3: sam-test3.c
	$(CC) $(CFLAGS) $< -lsam -o $@

sam-test4: sam-test4.c
	$(CC) $(CFLAGS) $< -lsam -o $@

sam-test5: sam-test5.c
	$(CC) $(CFLAGS) $< -lsam -o $@

sam-test6: sam-test6.c
	$(CC) $(CFLAGS) $< -lsam -o $@

sam-test7: sam-test7.c
	$(CC) $(CFLAGS) $< -lsam -lcmap -pthread -o $@

sam-test8: sam-test8.c
	$(CC) $(CFLAGS) $< -lsam -lcmap -o $@

sam-test9: sam-test9.c
	$(CC) $(CFLAGS) $< -lsam -lcmap -o $@

stdin-to-usock: stdin-to-usock.c
	$(CC) $(CFLAGS) $< -o $@

usock-to-stdout: usock-to-stdout.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(CMAP-TESTS-BIN) $(CONFDB-TESTS-BIN) $(CPG-TESTS-BIN) $(UTILS-BIN) $(SAM-TESTS-BIN)
