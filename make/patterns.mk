%.a:
	@$(AR) $(ARFLAGS) $@ $^
	@echo "Linking static library $@"

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Building C object file $@"

%: %.c
	@$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)
	@echo "Building C executable $@"

