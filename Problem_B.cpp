#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <string>

using namespace std;

/* Total number of customers. */
const int NCUSTOMERS = 10;

/* Global Variables. */

int buyingNum = 0;  //Variable containing number of tickets purchased (available to all threads)
int readyToEnter = 0;   //Number of threads ready to enter the stadium
int enteringNum = 0;    //Number which points to the index in enteringOrder which should be entered next.
int* enteringOrder; //Array which gives order to customer entering. Contains a customer id at each element.

pthread_cond_t* enterCond;  //Array of pthread conditions used for entering the stadium. Dynamically contains a condition for every thread.
pthread_mutex_t Out = PTHREAD_MUTEX_INITIALIZER;    //Original mutex for println function
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  //Mutex for allowing customers to arrive
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER; //Mutex for allowing customers to purchase tickets
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER; //Mutex for allowing customers to enter stadium

/* Insure that each output line has exclusive control of console. */

void println (string s)
{
    pthread_mutex_lock (&Out);
    cout << s << endl;
    pthread_mutex_unlock (&Out);
}

class Stadium
{
private:
    int customersArrived;
    int maxCustomers;
    int numberSeats;
    int numberSold;
    const static int baseTicket = 678;
    int admittingTicket;
    int nextTicket;
    bool cashierPresent;

public:
    Stadium (int, int);
    void close ();
    void open ();
    int buyTicket (int);
    void enterStadium (int, int);
    bool getCashierPresent ();
    void arriveCustomer();
    int getCustomersArrived();
    int getAdmittingTicket();
    int getNumberSold();
};

Stadium::Stadium (int maxC, int numS)
{
    maxCustomers = maxC;
    numberSeats = numS;
    customersArrived = 0;
    numberSold = 0;
    cashierPresent = false;
    nextTicket = baseTicket + 1;
    admittingTicket = baseTicket;
}

void Stadium::close ()
{
    cout << "Number of tickets sold: " << numberSold << endl;
}
void Stadium::open ()
{
    println("Stadium is opened.");
    cashierPresent = true;
}

int Stadium::getNumberSold()
{
    return numberSold;
}

void Stadium::arriveCustomer()
{
    customersArrived++;
}

int Stadium::buyTicket(int id)
{
    int ticket;
    ticket = baseTicket + numberSold;
    string sOut = "customer " + to_string(id) +
    " bought ticket " + to_string(ticket);
    println(sOut);
     numberSold++;
    return (ticket);
}

void Stadium::enterStadium (int id, int ticket)
{
    numberSeats--;
    string sOut;
    if ( numberSeats < 0 )
    sOut = "customer " + to_string(id) + " not admitted to stadium";
    else
    sOut = "customer " + to_string(id) +
    " admitted to stadium with ticket " + to_string(ticket);
    println (sOut);
    admittingTicket++;
    return;
}

bool Stadium::getCashierPresent()
{
    return cashierPresent;
}

int Stadium::getCustomersArrived()
{
    return customersArrived;
}

int Stadium::getAdmittingTicket()
{
    return admittingTicket;
}

class Customer
{
private:
    int id;
    Stadium *stadium;
    int ticket;
public:
    Customer ();
    Customer (int, Stadium*);
    void run ();
};

Customer::Customer()
{
    id = 0;
    stadium = NULL;
}

Customer::Customer ( int i, Stadium* s )
{
    id = i;
    stadium = s;
}

void Customer::run ()
{
    //Lock mutex while customer arrives
    pthread_mutex_lock(&mutex);

        string sOut = "Customer " + to_string(id) + " arrives at the Stadium.";
        println (sOut);
        stadium->arriveCustomer();

    //Unlock mutex, customer has completed arrival
    pthread_mutex_unlock(&mutex);

    //If all customers have arrived, unlock mutex2
    if(stadium->getCustomersArrived() == NCUSTOMERS)
    {
        printf("All customers have arrived.\n");
        pthread_mutex_unlock(&mutex2);
    }

    //Lock mutex2 during ticket sale. (mutex2 is initially locked until the final customer arrives and unlocks it)
    pthread_mutex_lock(&mutex2);

        while(stadium->getCashierPresent() == false){/*Wait for cashier to be present*/}
        ticket = stadium->buyTicket(id);
        enteringOrder[buyingNum] = id;  //Record the purchase order of customers purchase.
        buyingNum++;

    //Unlock mutex, sale complete.
    pthread_mutex_unlock(&mutex2);


    //Lock mutex, begin entering phase.
    pthread_mutex_lock(&mutex3);

    readyToEnter++; //Increment readyToEnter
    if(readyToEnter != NCUSTOMERS)  //Not all customers are ready to begin entering
    {
        pthread_cond_wait(&enterCond[id], &mutex3); //Wait until signaled to enter
        stadium->enterStadium(id,ticket);   //Enter the stadium
        enteringNum++;  //Increment enteringNum
        if(enteringNum < NCUSTOMERS )   //If there are more customers that need to enter, signal the next one.
        {
            pthread_cond_broadcast(&enterCond[enteringOrder[enteringNum]]);
        }
        pthread_mutex_unlock(&mutex3);  //Release mutex to the next customer.
    }
    else    //All customers are ready to begin entering
    {
        printf("All customers have bought tickets.\n");

        if(enteringOrder[0] == id)  //If the first customer to enter
        {
            stadium->enterStadium(id,ticket);   //Enter the stadium
            enteringNum++;  //Increment enteringNum
            if(enteringNum < NCUSTOMERS)   //If there are more customers that need to enter, signal the next one.
            {
                pthread_cond_broadcast(&enterCond[enteringOrder[enteringNum]]);
            }
            pthread_mutex_unlock(&mutex3);  //Release mutex to the next customer
        }
        else    //If not the first customer to enter
        {
            pthread_cond_broadcast(&enterCond[enteringOrder[enteringNum]]); //Signal the first customer to enter.
            pthread_cond_wait(&enterCond[id], &mutex3); //Wait to be signaled and release mutex to first customer to enter

            stadium->enterStadium(id,ticket);   //Enter the stadium
            enteringNum++;  //Increment enteringNum
            if(enteringNum < NCUSTOMERS)   //If there are more customers to enter the stadium, signal the next customer.
            {
                pthread_cond_broadcast(&enterCond[enteringOrder[enteringNum]]);
            }
            pthread_mutex_unlock(&mutex3);  //Release mutex to the next customer
        }
    }
}

void *callCrun (void *c)
{
    Customer *customer = (Customer *) c;
    customer->run();
    return NULL;
}

int main(int argc, const char * argv[])
{
    int err;
    Stadium* stadium = new Stadium (NCUSTOMERS, (NCUSTOMERS/2));
    pthread_mutex_lock(&mutex2);    //Initially lock mutex2
    Customer** customerPtrs;
    pthread_t* pthreads;

    customerPtrs = (Customer**)malloc(sizeof(Customer*) * NCUSTOMERS); //Dynamically allocate
    pthreads = (pthread_t*)malloc(sizeof(pthread_t) * NCUSTOMERS); //Dynamically allocate
    enterCond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t) * NCUSTOMERS); //Dynamically allocate
    enteringOrder = (int*)malloc(sizeof(int) * NCUSTOMERS); //Dynamically allocate memory

    for(int i = 0; i < NCUSTOMERS; i++)
    {
        Customer* newCustomer = new Customer (i, stadium);
        customerPtrs[i] = newCustomer;

        pthread_cond_t newCond;
        enterCond[i] = newCond;
        pthread_cond_init(&enterCond[i], NULL);

        err = pthread_create(&pthreads[i], NULL, callCrun, customerPtrs[i]);
        if(err)
        {
            printf("create of customer %d failed.", i);
            exit(2);
        }
    }

    //Open stadium
    stadium->open();

    //Wait for all threads to terminate.
    for(int i = 0; i < NCUSTOMERS; i++)
    {
        pthread_join(pthreads[i], NULL);
    }

    //Close stadium
    stadium->close();

    //Free dynamically allocated memory.
    free(enterCond);
    free(enteringOrder);
    free(pthreads);
    free(customerPtrs);
    return 0;
}
