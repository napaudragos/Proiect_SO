# 🗺️ Treasure Hunt Manager (Sisteme de Operare)

Proiect în **C pentru UNIX/Linux** care gestionează „vânători de comori”: fiecare hunt e un director cu comorile salvate într-un fișier binar. Pune în practică apelurile de sistem pentru fișiere, procese, semnale și pipe-uri.

## Componente
| Fișier | Rol |
| :--- | :--- |
| `treasure_manager.c` | CLI pentru adăugare, listare, vizualizare și ștergere de comori și hunt-uri. Lucrează cu `open/read/write`, ține un log per hunt și creează symlink-uri către el. |
| `treasure_hub.c` | Interfață interactivă. Pornește monitorul cu `fork` + `exec`, îi trimite comenzi prin **semnale** (`SIGUSR1`, `SIGTERM`) și citește răspunsurile printr-un **pipe**. |
| `monitor.c` | Proces în fundal care tratează semnalele cu `sigaction` și rulează `treasure_manager` pentru fiecare comandă primită. |
| `calculate_score.c` | Calculează scorul fiecărui utilizator dintr-un hunt. Hub-ul îl rulează în paralel pentru toate hunt-urile. |
| `gen_treasures.c` | Generează date de test. |

## Rulare
```bash
for f in treasure_manager treasure_hub monitor calculate_score gen_treasures; do gcc -o $f $f.c; done
./gen_treasures
./treasure_hub      # apoi: help, start_monitor, list_hunts, calculate_score, stop_monitor, exit
```
Manager-ul se poate folosi și direct, de exemplu `./treasure_manager --add hunt1` sau `./treasure_manager --list hunt1`.
