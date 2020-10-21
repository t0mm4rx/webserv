# COMMON =======================================================================
NAME = WebServer
#===============================================================================

# SOURCES ======================================================================
SOURCES_FOLDER = sources/
SOURCES =	WebServer.cpp \
			Socket.cpp \
			SocketManager.cpp \
			Server.cpp \
			Error.cpp
#===============================================================================

# INCLUDES =====================================================================
INCLUDES_FOLDER = includes/
INCLUDES = 	WebServer.hpp \
			Socket.hpp \
			Server.hpp \
			SocketManager.hpp

INCLUDES_PREFIXED = $(addprefix $(INCLUDES_FOLDER), $(INCLUDES))
#===============================================================================

# OBJECTS ======================================================================
OBJECTS_FOLDER = objects/

OBJECT = $(SOURCES:.cpp=.o)
OBJECTS = $(addprefix $(OBJECTS_FOLDER), $(OBJECT))
#===============================================================================

# FLAGS ========================================================================
FLAGS = -Wall -Wextra -Werror -std=c++98
#===============================================================================

DEBUG_VALUE = 0

$(OBJECTS_FOLDER)%.o :	$(SOURCES_FOLDER)%.cpp $(INCLUDES_PREFIXED)
	@mkdir -p $(OBJECTS_FOLDER)
	@echo "Compiling : $<"
	@clang++ $(FLAGS) -D DEBUG_ACTIVE=$(DEBUG_VALUE) -c $< -o $@

$(NAME): $(OBJECTS)
	@echo "Create    : $(NAME)"
	@clang++ $(FLAGS) $(OBJECTS) -o $(NAME)

all: $(NAME)

clean:
	@rm -rf $(OBJECTS_FOLDER)

fclean: clean
	@rm -rf $(NAME)

re: fclean all