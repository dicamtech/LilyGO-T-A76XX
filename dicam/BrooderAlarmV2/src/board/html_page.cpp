#include <stdint.h>

// HTML for the captive portal
const char* captivePortalPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Set APN</title>
  <style>
    /* General Body Styling */
    body {
      font-family: 'Arial', sans-serif;
      background-color: #f4f7fa;
      margin: 0;
      padding: 0;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      color: #333;
    }

    /* Center the form */
    .container {
      background-color: #fff;
      padding: 30px;
      border-radius: 8px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
      width: 100%;
      max-width: 400px;
    }

    /* Heading Styling */
    h2 {
      text-align: center;
      color: #4CAF50;
      font-size: 24px;
      margin-bottom: 20px;
    }

    /* Label Styling */
    label {
      font-size: 16px;
      margin-bottom: 5px;
      display: block;
    }

    /* Input Field Styling */
    input[type="text"],
    input[type="password"] {
      width: 100%;
      padding: 12px;
      margin: 8px 0;
      border: 1px solid #ccc;
      border-radius: 4px;
      box-sizing: border-box;
      font-size: 16px;
      background-color: #f9f9f9;
      transition: border-color 0.3s ease;
    }

    input[type="text"]:focus,
    input[type="password"]:focus {
      border-color: #4CAF50;
      outline: none;
    }

    /* Button Styling */
    button {
      width: 100%;
      padding: 12px;
      background-color: #4CAF50;
      color: white;
      border: none;
      border-radius: 4px;
      font-size: 16px;
      cursor: pointer;
      transition: background-color 0.3s ease;
    }

    button:hover {
      background-color: #45a049;
    }

    /* Responsive Design */
    @media screen and (max-width: 600px) {
      body {
        padding: 10px;
      }

      .container {
        padding: 20px;
      }

      h2 {
        font-size: 20px;
      }

      input[type="text"],
      input[type="password"] {
        padding: 10px;
        font-size: 14px;
      }

      button {
        font-size: 14px;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>Set APN Configuration</h2>
    <form action="/save" method="post">
      <label for="apn">APN:</label>
      <input type="text" id="apn" name="apn" required><br>

      <label for="username">Username:</label>
      <input type="text" id="username" name="username" required><br>

      <label for="password">Password:</label>
      <input type="password" id="password" name="password" required><br>

      <button type="submit">Save</button>
    </form>
  </div>
</body>
</html>
)rawliteral";
