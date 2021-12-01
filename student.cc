#include "student.h"
#include "MPRNG.h"
#include "nameServer.h"
#include "WATCardOffice.h"
#include "groupOff.h"
#include "printer.h"

extern MPRNG mprng;

Student::Student( Printer & prt, NameServer & nameServer, WATCardOffice & cardOffice, Groupoff & groupoff, unsigned int id, unsigned int maxPurchases )
: printer(prt), nameServer(nameServer), cardOffice(cardOffice), groupoff(groupoff),
id(id), maxPurchases(maxPurchases) {}


void Student::main() {
  numPurchases = mprng(1, maxPurchases);
#ifdef DEBUG
cout << endl << "studnets" <<  id << ": mprng used - numPurchase" << endl;
#endif
  favFlavour = static_cast<VendingMachine::Flavours>(mprng(0, 3));
#ifdef DEBUG
cout << endl << "studnets" <<  id << ": mprng used - amount" << endl;
#endif
  watCard = cardOffice.create(id, 5);
  giftCard = groupoff.giftCard();
  vm = nameServer.getMachine(id);

  printer.print(Printer::Kind::Student, id, 'S', favFlavour, numPurchases);

  printer.print(Printer::Kind::Student, id, 'V', vm->getId());

  unsigned int purchased = 0;
  for ( ;purchased < numPurchases; ) {
  #ifdef DEBUG
    cout << "student mprng yield" << endl;
  #endif
    yield(mprng(1, 10));


    bool isGiftCardPurchased = false;
    try {
      _Select (giftCard) {
        isGiftCardPurchased = true;
        vm->buy( favFlavour, *giftCard );
        printer.print(Printer::Kind::Student, id, 'G', favFlavour, (*giftCard).getBalance());
        giftCard.reset();
      }
      or _Select (watCard) {
        vm->buy( favFlavour, *watCard );
        printer.print(Printer::Kind::Student, id, 'B', favFlavour, (*watCard).getBalance());
      }
      purchased++;
    } catch (WATCardOffice::Lost&) {
      printer.print(Printer::Kind::Student, id, 'L');
      watCard = cardOffice.create(id, 5);
    } catch (VendingMachine::Funds&) {
      watCard = cardOffice.transfer(id, 5 + vm->cost(), watCard());
    } catch (VendingMachine::Stock&) {
      vm = nameServer.getMachine(id);
      printer.print(Printer::Kind::Student, id, 'V', vm->getId());
    } catch (VendingMachine::Free&) {
      isGiftCardPurchased
        ? printer.print(Printer::Kind::Student, id, 'a', favFlavour, (*giftCard).getBalance())
        : printer.print(Printer::Kind::Student, id, 'A', favFlavour, (*watCard).getBalance());
      yield(4);
    } // try
  }
  printer.print(Printer::Kind::Student, id, 'F');
}

