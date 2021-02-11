function C() {
    x=$(echo "000$1" | sed -e "s/0001/1/g" -e "s/000/0/g" )
    echo "print(hex(0b$x))" | python -
}
