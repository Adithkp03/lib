echo "Assignment 1"
echo "Address Book"

while true;
 do
    echo "1.Create"
    echo "2.Insert"
    echo "3.Display"
    echo "4.Update"
    echo "5.Delete"
    echo "6.Exit"

    echo "Enter the choice:"
    read -r choice || break

    case "$choice" in

    1)
        if [ ! -f Database.txt ]
        then
            touch Database.txt
            echo "File created successfully"
        else
            echo "File already exists"
        fi
    ;;

    2)
        read -r -p "Enter Employee ID: " empid || break

        if grep -q "^$empid " Database.txt
        then
            echo "Employee ID already exists!"
        else
            read -r -p "Enter Name: " name || break
            read -r -p "Enter City: " city || break
            read -r -p "Enter Mobile: " mobile || break

            echo "$empid $name $city $mobile" >> Database.txt
            echo "Data inserted successfully"
        fi
    ;;

    3)
        cat Database.txt
    ;;

    4)
        read -r -p "Enter Employee ID to update: " empid || break

        if grep -q "^$empid " Database.txt
        then
            grep -v "^$empid " Database.txt > temp

            read -r -p "Enter New Name: " name || break
            read -r -p "Enter New City: " city || break
            read -r -p "Enter New Mobile: " mobile || break

            echo "$empid $name $city $mobile" >> temp
            mv temp Database.txt

            echo "Data updated successfully"
        else
            echo "Employee ID not found!"
        fi
    ;;

    5)
        read -r -p "Enter Employee ID to delete: " empid || break

        if grep -q "^$empid " Database.txt
        then
            grep -v "^$empid " Database.txt > temp
            mv temp Database.txt

            echo "Data deleted successfully"
        else
            echo "Employee ID not found!"
        fi
    ;;

    6)
        exit 0
    ;;

    *)
        echo "Invalid choice"
    ;;

    esac
done
