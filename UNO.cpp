#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <stack>
#include <list>
#include <ctime>

//clase para carta
class card {
private:
    std::string type;
    std::string color;
    int number;

public:
    //constructores de clase carta
    card() : color("none"), type("none"), number(-1) {}
    card(std::string t, std::string c, int n) : type(t), color(c), number(n) {}

    //getters para poder acceder a atributos privados
    std::string getType() const { return type; }
    std::string getColor() const { return color; }
    int getNumber() const { return number; }

    //metodo para imprimir la carta
    std::string getFullName() const {
        if (type == "common") {
            return color + " " + std::to_string(number);
        } else if (type == "wildcard" || type == "+4 wildcard") {
            return type;
        } else {
            return color + " " + type;
        };
    };
};

//clase para baraja original, usa arreglos dinámicos
class deck {
private:
    std::vector<card> cards;

public:
//constructor baraja, crea las cartas y las agrega al arreglo
    deck() {
        std::vector<std::string> colors = {"red", "blue", "green", "yellow"};

        for (const auto& color : colors) {

            cards.push_back(card("common", color, 0));

            for (int j = 1; j < 10; ++j) {
                cards.push_back(card("common", color, j));
                cards.push_back(card("common", color, j));
            };

            for (int k = 0; k < 2; ++k) {
                cards.push_back(card("reverse", color, -1));
                cards.push_back(card("skip", color, -1));
            };
        };

        for (int k = 0; k < 4; k++) {
            cards.push_back(card("wildcard", "none", -1));
            cards.push_back(card("+4 wildcard", "none", -1));
        };
    };

    //mezcla la baraja, usa función de para randomizar
    void shuffle() {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::shuffle(cards.begin(), cards.end(), rng);
    };

    std::vector<card> getCards() const {
        return cards;
    };
};

//clase para las barajas principal y de descarte, usa pilas
class cardStack {
private:
    std::stack<card> s;

public:
    cardStack() {}
//metodo para transformar el arreglo en pila
    void transform(const std::vector<card>& v) {
        for (const auto& c : v) {
            s.push(c);
        };
    };
    //metodo para tomar tope de la pila guardarlo y eliminarlo de esta
    card pop() {
        if (s.empty()) {
            return card();
        };
        card top = s.top();
        s.pop();
        return top;
    };
    //metodo para retornar el valor del tope sin eliminarlo
    card getTop() const {
        return s.top();
    };
    //metodo para agregar el valor al tope de la pila
    void push(const card& c) {
        s.push(c);
    };
    //¿Está vacia?
    bool isEmpty() const {
        return s.empty();
    };
    //tamaño de la pila
    int size() const {
        return s.size();
    };
};


//clase para el jugador, tiene nombre y el mazo
class player {
public:
    std::string name;
    std::vector<card> hand;

public:
//constructor de jugador
    player() {}
    player(std::string n) : name(n) {}

    //metodo para armar el mazo con 7 cartas de la baraja principal
    void pick(cardStack& deck) {
        for (int i = 0; i < 7; i++) {
            if (!deck.isEmpty()) {
                hand.push_back(deck.pop());
            };
        };
    };

    //metodo para eliminar x carta del mazo
    void removeCard(const card& c) {
        for (auto it = hand.begin(); it != hand.end(); ++it) {
            if (it->getFullName() == c.getFullName()) {
                hand.erase(it);
                return;
            };
        };
    };
    //metodo para mostrar mazo
    void showhand(){
      for(int i=0; i<hand.size(); i++){
        std:: cout << i+1 << ')'<< hand[i].getFullName() << "\n";
      };
    };
};

//clase principal que controla la lógica del juego
class Game {
private:
    deck initialDeck;
    cardStack mainStack;
    cardStack discardStack;
    std::list<player> players;  //estructura de una libreria ya existente, es una lista doblemente enlazada, se puede usar sus apuntadores para que actue como cola circular
    std::list<player>::iterator currentPlayer;
    int direction; //bandera dirección de turnos
    std::string activeColor; //variable status actual del color
    bool gameOver; //bandera para saber si el juego debe terminar
    std::string username;
    bool userP= false; //bandera para diferenciar el usuario de la(s) cpu

public:
    Game() : direction(1), gameOver(false) {}

    
    void user(char x){
      if(x=='s'){
        std::cout << "Ingrese su nombre: ";
        std:: string name;
        std::cin >> name;
        username = name;
        player p(username);
        p.pick(mainStack);
        players.push_back(p);
        userP= true;
      };
    };
    //flujo principal del juego
    void run() {
        std::cout << "\n¡Comienza el juego!\n";
        if(userP){
          while (!gameOver){
            player& currentP = *currentPlayer;
            if(currentP.name == username){
              playUser();
            }else{
              playTurn();
            };
          };
        }else{
          while (!gameOver) {
            playTurn();
          }
        };
        std::cout << "\n=== Fin del juego ===\n";
    };

    //inicio de juego

    //metodo para crear arreglo, mezclarlo y apilarlo
    void initializeGame(int numPlayers, char x) {
        initialDeck.shuffle();
        mainStack.transform(initialDeck.getCards());
        user(x);
        //crea uno a uno los jugadores, usa la funcion pick para darle sus 7 cartas y los agrega a la cola circular
        for (int i = 0; i < numPlayers; ++i) {
            player p("CPU_" + std::to_string(i + 1));
            p.pick(mainStack);
            players.push_back(p);
        };
        currentPlayer = players.begin();

        //se saca la primera carta hasta que no sea un comodín, para inicializar la baraja de descarte
        card firstCard = mainStack.pop();
        while (firstCard.getType() == "wildcard" || firstCard.getType() == "+4 wildcard") {
            mainStack.push(firstCard);
            mainStack.pop();
        };
        discardStack.push(firstCard);
        activeColor = firstCard.getColor();

        std::cout << "\nLa primera carta en la mesa es: " << firstCard.getFullName() << "\n";

        //se llama el metodo para manejar si la primera carta es especial
        handleInitialCard(firstCard);
    };

private:
    //lógica para un turno completo de cpu
    void playTurn() {
        player& currentP = *currentPlayer;
        card topCard = discardStack.getTop();

        std::cout << "\n===== Turno de " << currentP.name << " =====\n";
        std::cout << "Mano del jugador (" << currentP.hand.size() << " cartas)\n";
        std::cout << "Carta en la mesa: " << topCard.getFullName() << " (Color activo: " << activeColor << ")\n";

        //buscar una carta jugable
        int playableIndex = canPlay(currentP, topCard);

        if (playableIndex == -1) {
            //si no puede jugar, roba una carta
            std::cout << "-> " << currentP.name << " no tiene jugada y roba una carta.\n";
            stealCard(currentP);
        } else {
            //si puede jugar, la pone en la pila de descarte
            card cardToPlay = currentP.hand[playableIndex];
            currentP.removeCard(cardToPlay);
            discardStack.push(cardToPlay);
            std::cout << "-> " << currentP.name << " juega: " << cardToPlay.getFullName() << "\n";

            //se llama metodo para manejar efectos de la carta jugada
            handleCardEffect(cardToPlay);
        };

        //se llama metodo que verifica si el jugador actual ganó
        checkWinner(currentP);
        if (gameOver) return;

        //se llama metodo para avanzar en los turnos
        updateTurn();
    };

    //lógica para un turno completo de usuario
    void playUser() {
        player& currentP = *currentPlayer;
        card topCard = discardStack.getTop();

        std::cout << "\n==////== Turno de " << currentP.name << " ==////==\n";
        std::cout << "Tu mano (" << currentP.hand.size() << " cartas)\n";
        currentP.showhand();
        std::cout << "Carta en la mesa: " << topCard.getFullName() << " (Color activo: " << activeColor << ")\n";

        // Buscar una carta jugable
        int playableIndex = canPlay(currentP, topCard);

        if (playableIndex == -1) {
            // Si no puede jugar, roba una carta
            std::cout << "-> " << currentP.name << " no tiene jugada y roba una carta.\n";
            stealCard(currentP);
        } else {
            // Si puede jugar:
            int i;
            std::cout << "Elija una carta para jugar: \n";
            std::cin >> i;
            const card& c = currentP.hand[i-1];
            // Un comodín siempre se puede jugar
            if ((c.getType() == "wildcard" || c.getType() == "+4 wildcard") || (c.getColor() == activeColor) || c.getNumber() != -1 && c.getNumber() == topCard.getNumber() || (c.getType() != "common" && c.getType() == topCard.getType())){
              card cardToPlay = currentP.hand[i-1];
              currentP.removeCard(cardToPlay);
              discardStack.push(cardToPlay);
              std::cout << "-> " << currentP.name << " juega: " << cardToPlay.getFullName() << "\n";
              handleCardEffect(cardToPlay);
            }else{
              std::cout << " no puede jugar esa carta.\n";
              playUser();
            };
        };

        checkWinner(currentP);
        if (gameOver) return;

        updateTurn();
    };

    //metodo para manejar efecto en la primera carta
    void handleInitialCard(const card& c) {
        if (c.getType() == "skip") {
            std::cout << " " << currentPlayer->name << " pierde el primer turno.\n";
            updateTurn();
        } else if (c.getType() == "reverse") {
            std::cout << " ¡El juego empieza en reversa!\n";
            direction *= -1;
            //en el inicio, el primer jugador es el anterio
            currentPlayer = players.end();
            --currentPlayer;
        };
    };
  //metodo para manejar efectos de cartas
    void handleCardEffect(const card& c) {
        //se actualiza el status de active Color si es un comodín
        if (c.getType() == "wildcard" || c.getType() == "+4 wildcard") {
            activeColor = chooseBestColor(*currentPlayer);
            std::cout << " Color cambiado a: " << activeColor << "\n";
        } else {
            activeColor = c.getColor();
        };
        if (c.getType() == "reverse") {
            direction *= -1;
            std::cout << " Sentido del juego invertido.\n";
        } else if (c.getType() == "skip") {
            std::cout << " Se salta el turno del siguiente jugador.\n";
            updateTurn(); // Salta un turno extra
        } else if (c.getType() == "+4 wildcard") {
            std::cout << "+4 El siguiente jugador roba 4 cartas y pierde su turno.\n";
            updateTurn();
            for (int i = 0; i < 4; ++i) {
                stealCard(*currentPlayer);
            };
        };
    };

    //lógica para saber si una carta del mazo sirve o no respecto a la carta del tope de la pila de descarte
    int canPlay(const player& p, const card& topCard) const {
        for (int i = 0; i < p.hand.size(); ++i) {
            const card& c = p.hand[i];
            //comodín siempre se puede jugar
            if (c.getType() == "wildcard" || c.getType() == "+4 wildcard") return i;
            //coincide con el color activo
            if (c.getColor() == activeColor) return i;
            //si ambas son numéricas
            if (c.getNumber() != -1 && c.getNumber() == topCard.getNumber()) return i;
            //skip, reverse
            if (c.getType() != "common" && c.getType() == topCard.getType()) return i;
        };
        // No se encontró carta jugable
        return -1;
    };

    //metodo para elegir el color que más cartas tienen en el mazo
    std::string chooseBestColor(const player& p) {
        std::vector<std::string> colors = {"red", "blue", "green", "yellow"};
        std::vector<int> counts(4, 0);
        for (const auto& c : p.hand) {
            for (int i = 0; i < colors.size(); ++i) {
                if (c.getColor() == colors[i]) counts[i]++;
            };
        };
        int max = 0;
        for (int i = 1; i < counts.size(); ++i) {
            if (counts[i] > counts[max]) max = i;
        };
        //si no tiene cartas de color elige al azar
        return counts[max] > 0 ? colors[max] : colors[rand() % 4];
    };

    //metodo para barajar el descarte si la baraja principal queda vacía
    void reshuffle() {
        if (!mainStack.isEmpty()) return;
        std::cout << "\n--- ¡El mazo se acabó! Barajando la pila de descarte... ---\n";
        card top = discardStack.pop(); //guardar la carta de arriba
        std::vector<card> toShuffle;
        while (!discardStack.isEmpty()) {
            toShuffle.push_back(discardStack.pop());
        };
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(toShuffle.begin(), toShuffle.end(), g);
        mainStack.transform(toShuffle);
        discardStack.push(top); //poner la carta guardada
    };
    //metodo para robar carta
    void stealCard(player& p) {
        reshuffle();
        if (!mainStack.isEmpty()) {
            p.hand.push_back(mainStack.pop());
        } else {
            std::cout << "No hay cartas para robar.\n";
        };
    };
    //metodo que revisa si un jugador ganó
    void checkWinner(const player& p) {
        if (p.hand.empty()) {
            std::cout << "\n🎉 ¡" << p.name << " se ha quedado sin cartas y gana el juego! 🎉\n";
            gameOver = true;
        };
    };

    //metodo que maneja el avance del turno dependiendo de la dirección
    void updateTurn() {
        if (direction == 1) { //derecha
            ++currentPlayer;
            if (currentPlayer == players.end()) {
                currentPlayer = players.begin();
            };
        } else { //izquierda
            if (currentPlayer == players.begin()) {
                currentPlayer = players.end();
            };
            --currentPlayer;
        };
    };
};

//main
int main() {
    srand(static_cast<unsigned int>(time(nullptr)));
    //se crea el game
    Game game;
    int numPlayers;
    char x;

    std::cout << "====================\n";
    std::cout << "  SIMULADOR DE UNO  \n";
    std::cout << "====================\n";
    std::cout << "Ingrese el número de jugadores CPU (2-6): ";
    std::cin >> numPlayers;
    std::cout << "Desea jugar? (s/n) ";
    std::cin >> x;
    if (numPlayers < 2 || numPlayers > 6) {
        std::cout << "Número de jugadores no válido.\n";
        return 1;
    };

    game.initializeGame(numPlayers,x);
    game.run();

    return 0;
};
