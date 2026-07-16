# Testing `lottery_on_sepolia.sol` on Remix

Instructions to deploy and manually test the `Lottery` contract (`src/Lottery.sol`) on the Sepolia testnet using Remix IDE and MetaMask.

## Prerequisites

- [MetaMask](https://metamask.io/) browser extension installed, with an account added.
- MetaMask network switched to **Sepolia** (enable "Show test networks" in MetaMask settings if it's not listed).
- Sepolia test ETH in that account. Get some from a faucet, e.g.:
  - https://sepoliafaucet.com/
  - https://www.alchemy.com/faucets/ethereum-sepolia
  - https://cloud.google.com/application/web3/faucet/ethereum/sepolia
- A second MetaMask account (or a second browser profile/wallet) recommended, so you can test `enter()` as a non-manager player as well as the manager.

## 1. Open Remix and load the contract

1. Go to https://remix.ethereum.org.
2. In the File Explorer, create a new file, e.g. `Lottery.sol`.
3. Copy the full contents of `src/Lottery.sol` into it.

## 2. Compile

1. Open the **Solidity Compiler** tab (left sidebar).
2. Set the compiler version to match the pragma (`0.8.x`).
3. Click **Compile Lottery.sol**. Confirm there are no errors.

## 3. Deploy to Sepolia

1. Open the **Deploy & Run Transactions** tab.
2. Set **Environment** to **Injected Provider - MetaMask**. MetaMask will prompt to connect — approve it, and confirm it's on Sepolia.
3. Select the `Lottery` contract from the dropdown.
4. Click **Deploy** and confirm the transaction in MetaMask.
5. Once mined, the deployed contract appears under "Deployed Contracts". Note its address — this is the account that ran `deploy` and becomes `manager` (set in the constructor).

smart-contract:
- https://sepolia.etherscan.io/address/0x3d5983543381dFEd2Dd41d9292Ba86c58C582Be7

## 4. Test `enter()` as a player

`enter(uint temp, uint hum)` is `payable` and requires:
- `msg.value > 0.01 ether` e.g. 11 finney
- `temp < 40`
- `hum > 0`

transactions:
- enter: 0xc7fa3aca5089c33f2cbe7f4175e3886c397e84fcb8dded94a46a8829ae62af1b
- enter: 0x4817ad6d555134bcd08d8df6949f715eb40a2a961fa9053ec5ff0c60bb55d0eb
- Pick Winner: 0x4f999ec1b85e2d48f9f505355941495cfe3a2999654baf1af703f28ae5ef8339

To find the winner:

1. Open the transaction on Sepolia Etherscan:
   `https://sepolia.etherscan.io/tx/0x4f999ec1b85e2d48f9f505355941495cfe3a2999654baf1af703f28ae5ef8339`
2. Look for the **"Internal Transactions"** tab on that page — it will show a transfer from the contract address to the winner's address, with the amount. 
Transfer 0.026 ETH From 0x3d5983543381dfed2dd41d9292ba86c58c582be7 To 0x54269a725ca37d9197db03c4573e4e520f9f26d7


To call it in Remix:
1. Expand the deployed contract instance.
2. Next to the **Value** field at the top of the Deploy tab, enter e.g. `0.02` and select unit `Ether`.
3. Expand `enter`, fill in `temp` (e.g. `25`) and `hum` (e.g. `60`).
4. Click **transact** and confirm in MetaMask.
5. Check `players(0)` (auto-generated getter for the public array) to confirm your address was pushed.

**Negative tests to try** (each should revert):
- Call `enter` with **Value** set to `0` or below `0.01 ether` → expect revert `"insufficient entry fee"`.
- Call `enter` with `temp = 45` → expect revert `"temperature too high"`.
- Call `enter` with `hum = 0` → expect revert `"invalid humidity"`.

Repeat the successful `enter()` call from a second MetaMask account (switch account in MetaMask, Remix will pick it up as the new `msg.sender`) so you have at least 2 players before testing `pickWinner`.

## 5. Check contract state

- `getContractBalance()` → should reflect the sum of ETH sent via successful `enter()` calls.
- `manager()` → should be the address that deployed the contract.
- `players(i)` → iterate `i = 0, 1, 2...` to list all current entrants.

## 6. Test `pickWinner()` access control

`pickWinner()` is restricted to `manager` and reverts if `players.length == 0`.

1. While still connected as the **manager** account (the deployer) in MetaMask/Remix, click **transact** on `pickWinner`.
2. Confirm it succeeds, transfers the full contract balance to the randomly selected winner, and resets `players` (verify with `getContractBalance()` returning `0` and `players(0)` reverting/empty afterward).

**Negative test:**
- Switch MetaMask to a **non-manager** account and call `pickWinner()` → expect revert (the `restricted` modifier's `require(msg.sender == manager)`).

**Edge case:**
- Right after a successful `pickWinner()` (players reset to empty), call `pickWinner()` again before any new `enter()` → expect revert `"no players"`.

## 7. Verify on Etherscan

Every transaction hash shown in Remix's terminal log can be pasted into `https://sepolia.etherscan.io/tx/<hash>` to confirm on-chain status, gas used, and emitted events/transfers — useful for cross-checking the winner payout.
