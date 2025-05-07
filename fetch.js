//TUTORIAL -> https://www.youtube.com/watch?v=eS-FVnhjvEQ
//Firebase stuff -> https://firebase.google.com/docs/web/learn-more?hl=en&authuser=4#modular-version

// Import Firebase from the CDN
import { initializeApp } from "https://www.gstatic.com/firebasejs/9.22.2/firebase-app.js";
import {
  getDatabase,
  ref,
  get,
  set,
  remove,
} from "https://www.gstatic.com/firebasejs/9.22.2/firebase-database.js";

// Your web app's Firebase configuration
const firebaseConfig = {
  apiKey: "AIzaSyBEaP90MeuJJOKCU3DfstvMPwW6f1PVvs0",
  authDomain: "rod-system.firebaseapp.com",
  databaseURL: "https://rod-system-default-rtdb.firebaseio.com",
  projectId: "rod-system",
  storageBucket: "rod-system.firebasestorage.app",
  messagingSenderId: "722670835307",
  appId: "1:722670835307:web:7b1260b82c4bf63200c32e",
  measurementId: "G-ZYSFT0E2P9",
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const database = getDatabase(app);

// Fetch data from Firebase
async function fetchRoomOccupancies() {
  const dbRef = ref(database, "rooms");
  try {
    const snapshot = await get(dbRef);
    if (snapshot.exists()) {
      const data = snapshot.val();
      return data;
    } else {
      console.log("No data available");
      return {};
    }
  } catch (error) {
    console.error("Error fetching data:", error);
    return {};
  }
}

function fetchRoomData(callback) {
  fetchRoomOccupancies()
    .then((data) => {

      // If the data is directly an object of rooms, no need for a "rooms" wrapper
      const rooms = data;

      // Convert the rooms object into an array and sort by room number
      const roomArray = Object.entries(rooms)
        .map(([roomNumber, roomData]) => ({
          room_number: roomNumber,
          isEmpty: roomData.isEmpty,
          isReserved: roomData.isReserved,
        }))
        .sort((a, b) => a.room_number - b.room_number);

      // Write the table body
      let tableBody = document.querySelector("#room_table tbody");
      let floor_one_output = "";
      let floor_two_output = "";
      const roomMap = new Map();
      roomArray.forEach((room) => {
        // Map room number to occupancy status
        const reservation_status = room.isReserved
          ? "Reserved"
          : "Not Reserved";
        // Check if the room is empty or occupied
        const occupancy = room.isEmpty ? "Vacant" : "Occupied";
        room.room_number = room.room_number.slice(0, 3); // Get the first 3 characters of the room number
        const roomNumberChecker = parseInt(room.room_number); // Convert the room number to an integer for comparison
        if (roomMap.has(roomNumberChecker)) {
          if (reservation_status === "Reserved") {
            roomMap.set(parseInt(room.room_number), reservation_status);
          } else {
            let previousOccupancy = roomMap.get(roomNumberChecker); // Get the previous occupancy status
            if (occupancy === "Occupied" && previousOccupancy === "Vacant") {
              roomMap.set(parseInt(room.room_number), occupancy);
            }
          }
        } else {
          if (reservation_status === "Reserved") {
            roomMap.set(parseInt(room.room_number), reservation_status);
          } else {
            roomMap.set(parseInt(room.room_number), occupancy);
          }
        }
      });
      //setting the tables for the two floors
      roomMap.forEach((occupancy, roomNumber) => {
        const row = `<tr><td>${roomNumber}</td><td>${occupancy}</td></tr>`;
        if (roomNumber < 200) {
          floor_one_output += row;
        } else {
          floor_two_output += row;
        }
      });

      // Update the table based on the current page
      if (window.location.pathname.includes("occupancies.html")) {
        tableBody.innerHTML = floor_one_output;
      }
      if (window.location.pathname.includes("floor_two.html")) {
        tableBody.innerHTML = floor_two_output;
      }

      // Pass the roomMap to the callback
      callback(roomMap);
    })
    .catch((error) => console.error("Error processing room data:", error));
}

const divs = document.querySelectorAll(".floorplan");
window.onload = () => {
  // Call fetchRoomData with a callback to handle roomMap after fetching
  function displayRoomData() {
    fetchRoomData((roomMap) => {
      fetchReservations().then((data) => {
        const reservations = data; // Call fetchReservations to get the reservation data
        divs.forEach((div) => {
          const rooms = div.getElementsByClassName("room");
          for (let i = 0; i < rooms.length; i++) {
            const room_num_text = rooms[i].getElementsByTagName("p")[0]; //get the first(and only) p element in the room and its text content
            const room_num = room_num_text.textContent; //create seperate variable to separate the room text of the room number and the actual number value
            const occ = roomMap.get(parseInt(room_num));
            if (occ === "Reserved") {
              rooms[i].style.backgroundColor = "cyan";
              room_num_text.style.backgroundColor = "cyan";
              var checkSize=0;
              Object.entries(reservations).forEach(([key, reservation]) => {
                const room_number = reservation.room.slice(4, 7); // Get the room number from the reservation data

                if (room_number === room_num) {
                  const label = document.createElement("div");
                  label.className = "end-time-label";
                  label.textContent = `Reservation Ends: ${reservation.endTime}`;
                  rooms[i].appendChild(label);
                }
                checkSize++;
              });
              if(checkSize === 0) {
                //set in the event a reservation is cancelled during its reservation time
                set(ref(database, "rooms/" + room_num), {
                  isEmpty: false,
                  isReserved: false,
                });
              }
            } else if (occ === "Occupied") {
              rooms[i].style.backgroundColor = "red";
              room_num_text.style.backgroundColor = "red";
              const existingLabels = rooms[i].querySelectorAll(".end-time-label");
              existingLabels.forEach(label => label.remove());
            } else if (occ === "Vacant") {
              rooms[i].style.backgroundColor = "green";
              room_num_text.style.backgroundColor = "green";
              const existingLabels = rooms[i].querySelectorAll(".end-time-label");
              existingLabels.forEach(label => label.remove());
            } else {
              rooms[i].style.backgroundColor = "white";
              room_num_text.style.backgroundColor = "white";
            }
            checkReservations();
          }
        });
      });
    });
  }
  async function fetchReservations() {
    const dbRef = ref(database, "reservations");
    try {
      const snapshot = await get(dbRef);
      if (snapshot.exists()) {
        const data = snapshot.val();
        return data;
      } else {
        console.log("No data available");
        return {};
      }
    } catch (error) {
      console.error("Error fetching data:", error);
      return {};
    }
  }
  function checkReservations() {
    fetchReservations().then((data) => {
      const reservations = data;
      fetchRoomOccupancies().then((rooms) => {
        const roomArray = Object.entries(rooms || {})
          .map(([roomNumber, roomData]) => ({
            room_number: roomNumber,
            isEmpty: roomData.isEmpty,
            isReserved: roomData.isReserved,
          }))
          .sort((a, b) => a.room_number - b.room_number);
        Object.entries(reservations).forEach(([key, reservation]) => {
          const room_num = reservation.room.slice(4, 7); // Get the room number from the reservation data
          const room_index = roomArray.findIndex(
            (room) => room.room_number.slice(0, 3) === room_num
          );
          if (reservation.date == new Date().toLocaleDateString()) {
            const now = new Date();
            const hours = now.getHours().toString().padStart(2, "0");
            const minutes = now.getMinutes().toString().padStart(2, "0");
            const militaryTime = `${hours}:${minutes}`; //javascript logs time in military time format
            if (
              reservation.startTime <= militaryTime &&
              reservation.endTime >= militaryTime
            ) {
              console.log("Time correct");
              roomArray[room_index].isReserved = true; // Set the room as reserved
              roomArray[room_index].isEmpty = false; // Set the room as occupied
            } else {
              roomArray[room_index].isReserved = false; // Set the room as not reserved
              if (
                militaryTime > reservation.endTime &&
                militaryTime > reservation.startTime
              ) {
                remove(ref(database, "reservations/" + key));
              }
            }
          } else {
            roomArray[room_index].isReserved = false; // Set the room as not reserved
            if (new Date().toLocaleDateString() > reservation.date) {
              if (reservation.endTime < militaryTime) {
                remove(ref(database, "reservations/" + key));
              }
            }
          }
          set(ref(database, "rooms/" + roomArray[room_index].room_number), {
            isEmpty: roomArray[room_index].isEmpty,
            isReserved: roomArray[room_index].isReserved,
          });
        });
      });
    });
  }
  displayRoomData();
  setInterval(displayRoomData, 10000);
};
