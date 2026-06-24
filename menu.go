package main

import (
	"errors"
	"fmt"
	"log"
)

func main() {
	log.SetPrefix("greetings: ")
	log.SetFlags(0)

	message, err := Hello("Daniel")

	if err != nil {
		log.Fatal(err)
	}

	fmt.Println(message)

}

func Hello(name string) (string, error) {
	if name == "" {
		return "", errors.New("empty name")
	}

	message := fmt.Sprintf("Hi %v.  Welcome!", name)
	return message, nil
}
