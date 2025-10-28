# Supaprastintas Blockchain — v0.1

--------------------------
Supaprastintas blockchain modelis parašytas C++. Demonstruoja pagrindines sąvokas: vartotojų generavimą, transakcijų sąrašą, blokų kasimą (Proof-of-Work) ir grandinės susiejimą per bloko hash.

-------------------------
**Kompiliuoti:** g++ -std=c++17 -O2 main.cpp -o blockchain <rm>

**Paleisti:** ./blockchain <rm>

--------------------------

## Struktūros ir klasės

**User** – saugo vartotojo duomenis: vardą, viešą raktą ir balansą.  
Naudojama vartotojų kūrimui ir balansų valdymui.

**Transaction** – aprašo vieną pervedimą tarp vartotojų.  
Turi siuntėją, gavėją, sumą ir automatiškai generuojamą `transaction_id` pagal hash.

**BlockHeader** – apima visą bloko antraštę: ankstesnio bloko hash, laiką, versiją, transakcijų hash, difficulty ir nonce.  
Metodas `to_string()` sujungia šiuos duomenis į tekstą, kuris vėliau hashuojamas.

**Block** – jungia `BlockHeader` ir transakcijų sąrašą.  
Metodas `compute_hash()` apskaičiuoja viso bloko hash ir naudojamas tikrinant vientisumą.

**Blockchain** – pagrindinė klasė valdanti visą sistemą.  
Atsakinga už:
- vartotojų ir transakcijų generavimą,  
- blokų formavimą ir kasimą (Proof-of-Work),  
- balansų atnaujinimą,  
- grandinės vientisumo palaikymą.

--------------------------

## Veikimo eiga

1. **Sukuriamas genesis blokas**  
   Pirmasis blokas grandinėje. Jo `previous_hash` yra tik nuliai, nes prieš jį dar nieko nėra.  

2. **Sugeneruojami vartotojai**  
   Programa sukuria vartotojus su atsitiktiniais balansais ir unikaliu viešu raktu (`pk_...`).  
   Jie naudojami kaip siuntėjai ir gavėjai transakcijose.

3. **Generuojamos transakcijos**  
   Kiekviena transakcija turi:
   - siuntėją,
   - gavėją,
   - pervedamą sumą,
   - automatiškai sukurtą `transaction_id` (pagal hash).  
   Šios transakcijos patenka į „pending“ sąrašą.

4. **Formuojamas naujas blokas**  
   Iš pending transakcijų paimama dalis (100).  
   Iš jų sukuriamas `BlockHeader`, kuriame yra:
   - ankstesnio bloko hash,
   - dabartinis laikas (timestamp),
   - transakcijų hash (visų ID sujungtas ir suhashuotas),
   - difficulty („000“),
   - nonce (pradžioje 0).

5. **Kasimas (Proof of Work)**  
   Programa keičia `nonce` reikšmę, kol `custom_hash(header.to_string())` prasideda simboliais `"000"`.  

6. **Bloko patvirtinimas ir pridėjimas**  
   Radus tinkamą hash:
   - transakcijos pritaikomos (balansai atnaujinami),
   - panaudotos transakcijos pašalinamos iš laukiančių,
   - blokas įtraukiamas į grandinę.

7. **Procesas kartojamas**, kol nelieka laukiančių transakcijų.

8. **Rezultatas**  
   Programa išveda informaciją apie iškastus blokus, kiek laiko užtruko kasimas, likusias transakcijas ir paskutinio bloko hash.

--------------------------

**Screenshot'ai:**

Pradžia: <rm>
![Start](1.jpeg)

Pabaiga: <rm>
![End](2.jpeg)



