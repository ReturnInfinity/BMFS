%.a:
	@echo "AR      $@"
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	@echo "CC      $@"
	$(CC) $(CFLAGS) -c $< -o $@

%: %.o
	@echo "LD      $@"
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)
