# TODO

THREADER: сейчас 1 поток создается, зачем джойниться, а нужно создать все потоки из массива, а потом массив джойнить (в цикле)

EXEC: порождать новый процесс в дочернем (потому что после вызова exec() вызвавший процесс завершается), чтобы основной остался и можно смотреть на код завершения или т.п.