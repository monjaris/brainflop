this program reads a char and writes it right back, forever,
until you send an EOF or newline breaks it out.
nothing fancy, just a mirror.

,                       read one byte into the cell
[
    .                   write it back out
    ,                   read the next one
]
