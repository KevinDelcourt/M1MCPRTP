# Programme pour executer le pre-processeur C(++)
CPP = g++
# Indicateurs a donner au compilateur
CFLAGS = -Wall -g
# Indicateurs a donner au pre-processeur
# CPPFLAGS =  -DTRACE_SOUHAITE -DTRACE_THD
# -DTRACE_SOUHAIT : afficher souhaits activites
# -DTRACE_THD : trace de la creation/fin des threads
# Indicateurs a donner a l'editeur de liens
LDFLAGS = -lpthread
# Commande pour détruire un fichier (par defaut rm -f)
RM = rm -f
# Repertoire sous lequel se trouvent les fichiers
# implantant les "moniteurs de Hoare"
REPMH = MONITEURS_HOARE/
# Renommer les fichiers utilises dans ce cas precis
HM = HoareMonitor
HC = HoareCondition
ER = error
LI = List

# Cible : Dependances
# Regle de production de la cible (precedee d'une tabulation)
#
pcv1 : prodConsov1.o
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) prodConsov1.o -o pcv1 PCMonitorv1.o $(REPMH)$(HC).o $(REPMH)$(HM).o $(REPMH)sem.o $(REPMH)$(LI).o $(REPMH)$(ER).o -lpthread

prodConsov1.o : prodConsov1.cpp PCMonitorv1.o
	$(CPP) $(CFLAGS) $(CPPFLAGS) prodConsov1.cpp -c

PCMonitorv1.o : PCMonitorv1.cpp $(REPMH)$(HM).o $(REPMH)$(HC).o
	$(CPP) $(CFLAGS) $(CPPFLAGS) PCMonitorv1.cpp -c 

$(REPMH)$(HM).o : $(REPMH)$(HM).cpp $(REPMH)$(HC).o $(REPMH)$(ER).o $(REPMH)sem.o $(REPMH)$(LI).o
	$(CPP) $(CFLAGS) $(REPMH)$(HM).cpp -c  -o $(REPMH)$(HM).o

$(REPMH)$(HC).o : $(REPMH)$(HC).cpp $(REPMH)$(HM).o $(REPMH)$(ER).o
	$(CPP) $(CFLAGS) $(REPMH)$(HC).cpp -c  -o $(REPMH)$(HC).o

$(REPMH)$(ER).o : $(REPMH)$(ER).cpp 
	$(CPP) $(CFLAGS) $(REPMH)$(ER).cpp -c -o $(REPMH)$(ER).o

$(REPMH)sem.o : $(REPMH)sem.cpp $(REPMH)$(ER).o 
	$(CPP) $(CFLAGS) $(REPMH)sem.cpp -c -o $(REPMH)sem.o

$(REPMH)$(LI).o : $(REPMH)$(LI).cpp
	$(CPP) $(CFLAGS) $(REPMH)$(LI).cpp -c -o $(REPMH)$(LI).o

# Option du make pour "nettoyer" (ici les .o et l'executable)
clean :
	$(RM) $(REPMH)*.o *.o pcv1

