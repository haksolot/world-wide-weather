# World Wide Weather
Projet CESI 2023 station météo embarqué (Arduino Uno)

# Prérequis

### <u>Software</u>

- Bibliothèque SoftwareSerial
- Bibliothèque ChainableLED
- Bibliothèque RTCLib
- Bibliothèque Adafruit_BME280
- Bibliothèque TinyGPS++
- Bibliothèque SD
- Bibliothèque EEPROM
### <u>Hardware</u>

- Capteur de luminosité (Pin Analogue A0)
- Capteur de Pression / Température / Hygrométrie (Pin compatible I2C)
- Module RTC (Pin compatible I2C)
- Module GPS (Pin RX 5, TX 6)
- SD Card Shield
- SD Card
- Base Shield
- Dual Button (Pin 2 et 3 ou D2 de la Base Shield)
- Chainable RGB LED (Pin 4 et 5 ou D4 de la Base Shield)
# Installation

Pour installer et exécuter ce projet, suivez ces étapes :

1. **Prérequis :** Vérifiez les prérequis matériels et logiciels (Installation des bibliothèques et connexion des modules à l'Arduino).

2. **Branchement :** Connectez l'Arduino à votre ordinateur via un port USB.

3. **Compilation et Téléversement :** 

   ```bash
   # Accédez au répertoire du projet
   cd /chemin/vers/le/projet

   # installer les librairies
   mingw32-make libraries
   
   # Compiler et téléverser
   mingw32-make libraries
   ```
   
Le lancement du Makefile permettra de compiler et téléverser le code source sur l'arduino
# Modes

Le projet propose 4 modes proposant différentes intractions avec les composants système ou l'utilisateur.
Ces modes sont changeables grâce à des pressions sur les deux boutons poussoirs (Rouge et Vert).
## Standard (Par défaut)
*Signalisé par une led verte*

Ce mode récupère les données de tous les capteurs et modules pour les écrire dans un fichier sur la carte SD.

Il est **accessible** :
- Depuis le mode **Economique** : Appuyez 5 secondes sur le bouton vert.
- Depuis le mode **Configuration** : Appuyez 5 secondes sur le bouton rouge.
- Depuis le mode **Maintenance** : Lorsque vous quittez le mode maintenance, le mode précédent est remis en place.
## Maintenance
*Signalisé par une led orange*

Ce mode désactive l'écriture des données et retransmet les données captées sur le port série.

Il est **accessible** :
- Depuis le mode **Normal** : Appuyez 5 secondes sur le bouton rouge.
## Economique
*Signalisé par une led bleue*

Ce mode réduit la captation et l'écriture de données en multipliant son intervalle d'action par 2. De plus, les données GPS sont récupérées une fois sur deux.

Il est **accessible** :
- Depuis le mode **Normal** : Appuyez 5 secondes sur le bouton vert.

## Configuration
*Signalisé par une led jaune*

Ce mode permet à l'utilisateur de modifier des variables locales du programme affectant l'utilisation et la récupération des données de chaque modules et capteurs.

Il est accessible en appuyant sur le bouton rouge au démarrage.

# Erreurs

Les capteurs ont put être mal branchés ou être non fonctionnels.

C'est la raison pour laquelle notre système signalera tout problème avec un affichage Led unique à chaque problème.

| Erreur | Affichage |
| --- | --- |
| Accès à l'horloge RTC | LED intermittente rouge et bleue (fréquence 1Hz, durée identique pour les 2 couleurs) |
| Accès aux données du GPS | LED intermittente rouge et jaune (fréquence 1Hz, durée identique pour les 2 couleurs) |
| Accès aux données capteur | LED intermittente rouge et verte (fréquence 1Hz, durée identique pour les 2 couleurs)  |
| Données capteurs incohérentes | LED intermittente rouge et verte (fréquence 1Hz, durée 2 fois plus longue pour le vert) |
| Carte SD pleine | LED intermittente rouge et blanche (fréquence 1Hz, durée identique pour les 2 couleurs) |
| Accès ou écriture SD | LED intermittente rouge et blanche (fréquence 1Hz, durée 2 fois plus longue pour le blanc)  |
