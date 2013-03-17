
default : all

all $(MAKECMDGOALS):
	$(MAKE) -C scenegraph $(MAKECMDGOALS)
	$(MAKE) -C worldbuilder $(MAKECMDGOALS)
