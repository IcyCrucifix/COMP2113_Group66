CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2

OBJS := main.o hero.o card.o deck.o battle.o buff.o save_load.o utils.o

all: slay_text

slay_text: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

main.o: main.cpp battle.h save_load.h utils.h
hero.o: hero.cpp hero.h buff.h
card.o: card.cpp card.h hero.h deck.h
deck.o: deck.cpp deck.h utils.h
battle.o: battle.cpp battle.h utils.h
buff.o: buff.cpp buff.h
save_load.o: save_load.cpp save_load.h utils.h
utils.o: utils.cpp utils.h

clean:
	rm -f $(OBJS) slay_text

.PHONY: all clean
