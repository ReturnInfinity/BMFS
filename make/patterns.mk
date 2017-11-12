%.a:
	@echo "AR $@"
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

%: %.o
	@echo "LINK $@"
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(Q).SILENT:
