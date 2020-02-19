from Operatore import Operatore

operatori = []
nomi_operatori = ['Vodafone', 'Tim','Wind']
porta = 5000

if __name__ == "__main__":
    for i in nomi_operatori:
        porta += 1
        operatori.append(Operatore(i, porta, 1, 0, 0))
