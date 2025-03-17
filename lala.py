import os
import time
import logging
import asyncio  # Import asyncio
from telegram import Update, ReplyKeyboardMarkup
from telegram.ext import Application, CommandHandler, MessageHandler, filters, CallbackContext, ConversationHandler
from telegram.helpers import escape_markdown

# Suppress HTTP request logs
logging.getLogger("httpx").setLevel(logging.WARNING)
logging.getLogger("telegram").setLevel(logging.WARNING)

# Logging Configuration
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s",
)

# Bot Configuration
TELEGRAM_BOT_TOKEN = '7064980384:AAGfNFTaf81DF3P4NLhHm0TRBSEV1XfBATw'  # Replace with your bot token
OWNER_USERNAME = "Riyahacksyt"  # Replace with your Telegram username (without @)
ALLOWED_GROUP_ID = -1002295161013  # Replace with your allowed group ID
MAX_THREADS = 1500  # Default max threads
max_duration = 150  # Default max attack duration

# Attack & Feedback System
attack_running = False
feedback_waiting = {}
attack_ban_list = {}

# Key System
keys = {}  # Stores keys and their expiration time
redeemed_users = {}  # Tracks users who have redeemed keys and their expiration time
redeemed_keys_info = {}  # Tracks which user redeemed which key

# Custom Keyboard for Regular Users
regular_user_keyboard = [['🚀 Start', '⚔️ Attack', '🔑 Redeem Key']]
regular_user_markup = ReplyKeyboardMarkup(regular_user_keyboard, resize_keyboard=True)

# Custom Keyboard for Owner
owner_keyboard = [
    ['🚀 Start', '⚔️ Attack', '🔑 Redeem Key'],
    ['⏱ Set Duration', '🧵 Set Threads', '🔑 Generate Key'],
    ['🗝️ Keys', '❌ Delete Key']
]
owner_markup = ReplyKeyboardMarkup(owner_keyboard, resize_keyboard=True)

# Conversation States
GET_DURATION = 1
GET_KEY = 2
GET_ATTACK_ARGS = 3
GET_SET_DURATION = 4
GET_SET_THREADS = 5
GET_DELETE_KEY = 6

# Check if bot is used in the allowed group
def is_allowed_group(update: Update):
    chat = update.effective_chat
    return chat.type in ['group', 'supergroup'] and chat.id == ALLOWED_GROUP_ID

# Check if the user is the owner
def is_owner(update: Update):
    return update.effective_user.username == OWNER_USERNAME

# Start Command
async def start(update: Update, context: CallbackContext):
    chat = update.effective_chat

    # If the user starts the bot in a private chat, send the main channel link
    if chat.type == "private":
        main_channel_link = "https://t.me/+wnHGZwkgKBo0ZDdl"  # Replace with your main channel link
        await update.message.reply_text(
            f"🌟 *Welcome!* 🌟\n\n"
            f"🔗 *Join our main channel for free 180 sec server hack:* {main_channel_link}\n\n"
            "*Use the bot in the allowed group for attacks!*",
            parse_mode='Markdown'
        )
        return

    # If the user starts the bot in the allowed group
    if not is_allowed_group(update):
        return

    message = (
        "*🔥 Welcome to the battlefield! 🔥*\n\n"
        "*Use ⚔️ Attack to start an attack!*\n\n"
        "*💥 Let the war begin!*"
    )

    # Show different keyboards for owner and regular users
    if is_owner(update):
        await update.message.reply_text(text=message, parse_mode='Markdown', reply_markup=owner_markup)
    else:
        await update.message.reply_text(text=message, parse_mode='Markdown', reply_markup=regular_user_markup)

# Generate Key Command - Start Conversation
async def generate_key_start(update: Update, context: CallbackContext):
    if not is_owner(update):
        await update.message.reply_text("❌ *Only the owner can generate keys!*", parse_mode='Markdown')
        return ConversationHandler.END

    await update.message.reply_text("⚠️ *Enter the duration for the key (e.g., 1H for 1 hour or 1D for 1 day).*", parse_mode='Markdown')
    return GET_DURATION

# Generate Key Command - Handle Duration Input
async def generate_key_duration(update: Update, context: CallbackContext):
    duration_str = update.message.text

    if not duration_str.endswith(('H', 'D')):  # H for hours, D for days
        await update.message.reply_text("❌ *Invalid format! Use H for hours or D for days (e.g., 1H or 1D).*", parse_mode='Markdown')
        return GET_DURATION

    # Generate a unique key
    unique_key = os.urandom(4).hex().upper()  # 4 bytes = 8 characters
    key = f"{OWNER_USERNAME}-{duration_str}-{unique_key}"
    keys[key] = time.time() + (int(duration_str[:-1]) * 3600 if duration_str.endswith('H') else int(duration_str[:-1]) * 86400)

    await update.message.reply_text(f"🔑 *Generated Key:* `{key}`\n\n*This key is valid for {duration_str}.*", parse_mode='Markdown')
    return ConversationHandler.END

# Redeem Key Command - Start Conversation
async def redeem_key_start(update: Update, context: CallbackContext):
    await update.message.reply_text("⚠️ *Enter the key to redeem.*", parse_mode='Markdown')
    return GET_KEY

# Redeem Key Command - Handle Key Input
async def redeem_key_input(update: Update, context: CallbackContext):
    key = update.message.text

    if key in keys and keys[key] > time.time():  # Check if key is valid and not expired
        user_id = update.effective_user.id
        redeemed_users[user_id] = keys[key]  # Store user's key expiration time
        redeemed_keys_info[key] = update.effective_user.username  # Track which user redeemed the key
        del keys[key]  # Remove the key from the available keys
        await update.message.reply_text(f"✅ *Key redeemed successfully! You can now use the attack command for {key.split('-')[1]}.*", parse_mode='Markdown')
    else:
        await update.message.reply_text("❌ *Invalid or expired key!*", parse_mode='Markdown')
    return ConversationHandler.END

# Attack Command - Start Conversation
async def attack_start(update: Update, context: CallbackContext):
    await update.message.reply_text("⚠️ *Enter the attack arguments: <ip> <port> <duration> <threads>*", parse_mode='Markdown')
    return GET_ATTACK_ARGS

# Attack Command - Handle Attack Input
async def attack_input(update: Update, context: CallbackContext):
    args = update.message.text.split()
    if len(args) != 4:
        await update.message.reply_text("❌ *Invalid input! Please enter <ip> <port> <duration> <threads>.*", parse_mode='Markdown')
        return GET_ATTACK_ARGS

    ip, port, duration, threads = args
    duration = int(duration)
    threads = int(threads)

    if duration > max_duration:
        await update.message.reply_text(f"❌ *Attack duration exceeds the max limit ({max_duration} sec)!*", parse_mode='Markdown')
        return GET_ATTACK_ARGS

    if threads > MAX_THREADS:
        await update.message.reply_text(f"❌ *Number of threads exceeds the max limit ({MAX_THREADS})!*", parse_mode='Markdown')
        return GET_ATTACK_ARGS

    await update.message.reply_text(
        f"⚔️ *Attack Started!*\n"
        f"🎯 *Target*: {ip}:{port}\n"
        f"🕒 *Duration*: {duration} sec\n"
        f"🧵 *Threads*: {threads}\n"
        f"🔥 *Let the battlefield ignite! 💥*",
        parse_mode='Markdown'
    )

    # Simulate attack completion after the duration
    await asyncio.sleep(duration)
    await update.message.reply_text("✅ *Attack Finished!*", parse_mode='Markdown')
    return ConversationHandler.END

# Set Duration Command - Start Conversation
async def set_duration_start(update: Update, context: CallbackContext):
    if not is_owner(update):
        await update.message.reply_text("❌ *Only the owner can set max attack duration!*", parse_mode='Markdown')
        return ConversationHandler.END

    await update.message.reply_text("⚠️ *Enter the maximum attack duration in seconds.*", parse_mode='Markdown')
    return GET_SET_DURATION

# Set Duration Command - Handle Duration Input
async def set_duration_input(update: Update, context: CallbackContext):
    global max_duration
    try:
        max_duration = int(update.message.text)
        await update.message.reply_text(f"✅ *Maximum attack duration set to {max_duration} seconds!*", parse_mode='Markdown')
    except ValueError:
        await update.message.reply_text("❌ *Invalid input! Please enter a number.*", parse_mode='Markdown')
        return GET_SET_DURATION
    return ConversationHandler.END

# Set Threads Command - Start Conversation
async def set_threads_start(update: Update, context: CallbackContext):
    if not is_owner(update):
        await update.message.reply_text("❌ *Only the owner can set max threads!*", parse_mode='Markdown')
        return ConversationHandler.END

    await update.message.reply_text("⚠️ *Enter the maximum number of threads.*", parse_mode='Markdown')
    return GET_SET_THREADS

# Set Threads Command - Handle Threads Input
async def set_threads_input(update: Update, context: CallbackContext):
    global MAX_THREADS
    try:
        MAX_THREADS = int(update.message.text)
        await update.message.reply_text(f"✅ *Maximum threads set to {MAX_THREADS}!*", parse_mode='Markdown')
    except ValueError:
        await update.message.reply_text("❌ *Invalid input! Please enter a number.*", parse_mode='Markdown')
        return GET_SET_THREADS
    return ConversationHandler.END

# Show Active Keys
async def show_keys(update: Update, context: CallbackContext):
    if not is_owner(update):
        await update.message.reply_text("❌ *Only the owner can view keys!*", parse_mode='Markdown')
        return

    active_keys = []
    for key, expiration_time in keys.items():
        if expiration_time > time.time():
            # Escape special characters in the key
            escaped_key = escape_markdown(key, version=2)
            active_keys.append(f"🔑 `{escaped_key}` (Expires in {int((expiration_time - time.time()) // 3600)} hours)")

    redeemed_keys = []
    for key, username in redeemed_keys_info.items():
        # Escape special characters in the key and username
        escaped_key = escape_markdown(key, version=2)
        escaped_username = escape_markdown(username, version=2)
        redeemed_keys.append(f"🔑 `{escaped_key}` (Redeemed by @{escaped_username})")

    if not active_keys and not redeemed_keys:
        await update.message.reply_text("❌ *No active or redeemed keys found!*", parse_mode='Markdown')
    else:
        message = "*🗝️ Active Keys:*\n" + "\n".join(active_keys) + "\n\n*🗝️ Redeemed Keys:*\n" + "\n".join(redeemed_keys)
        await update.message.reply_text(message, parse_mode='Markdown')

# Delete Key Command - Start Conversation
async def delete_key_start(update: Update, context: CallbackContext):
    if not is_owner(update):
        await update.message.reply_text("❌ *Only the owner can delete keys!*", parse_mode='Markdown')
        return ConversationHandler.END

    await update.message.reply_text("⚠️ *Enter the key to delete.*", parse_mode='Markdown')
    return GET_DELETE_KEY

# Delete Key Command - Handle Key Input
async def delete_key_input(update: Update, context: CallbackContext):
    key = update.message.text

    if key in keys:
        del keys[key]
        await update.message.reply_text(f"✅ *Key `{key}` deleted successfully!*", parse_mode='Markdown')
    elif key in redeemed_keys_info:
        del redeemed_keys_info[key]
        await update.message.reply_text(f"✅ *Redeemed key `{key}` deleted successfully!*", parse_mode='Markdown')
    else:
        await update.message.reply_text("❌ *Key not found!*", parse_mode='Markdown')
    return ConversationHandler.END

# Handle Photo Feedback
async def handle_photo(update: Update, context: CallbackContext):
    user_id = update.effective_user.id
    if user_id in feedback_waiting:
        del feedback_waiting[user_id]
        await update.message.reply_text("✅ *Thanks for your feedback!*", parse_mode='Markdown')

# Cancel Current Conversation
async def cancel_conversation(update: Update, context: CallbackContext):
    await update.message.reply_text("❌ *Current process canceled.*", parse_mode='Markdown')
    return ConversationHandler.END

# Handle Button Clicks
async def handle_button_click(update: Update, context: CallbackContext):
    query = update.message.text

    # Map button text to commands
    if query == '🚀 Start':
        await start(update, context)
    elif query == '⚔️ Attack':
        await attack_start(update, context)
    elif query == '⏱ Set Duration':
        await set_duration_start(update, context)
    elif query == '🧵 Set Threads':
        await set_threads_start(update, context)
    elif query == '🔑 Generate Key':
        await generate_key_start(update, context)
    elif query == '🔑 Redeem Key':
        await redeem_key_start(update, context)
    elif query == '🗝️ Keys':
        await show_keys(update, context)
    elif query == '❌ Delete Key':
        await delete_key_start(update, context)

# Main Bot Setup
def main():
    application = Application.builder().token(TELEGRAM_BOT_TOKEN).build()

    # Conversation Handlers
    generate_key_handler = ConversationHandler(
        entry_points=[CommandHandler("generatekey", generate_key_start), MessageHandler(filters.Text("🔑 Generate Key"), generate_key_start)],
        states={
            GET_DURATION: [MessageHandler(filters.TEXT & ~filters.COMMAND, generate_key_duration)],
        },
        fallbacks=[CommandHandler("cancel", cancel_conversation)],
    )

    redeem_key_handler = ConversationHandler(
        entry_points=[CommandHandler("redeemkey", redeem_key_start), MessageHandler(filters.Text("🔑 Redeem Key"), redeem_key_start)],
        states={
            GET_KEY: [MessageHandler(filters.TEXT & ~filters.COMMAND, redeem_key_input)],
        },
        fallbacks=[CommandHandler("cancel", cancel_conversation)],
    )

    attack_handler = ConversationHandler(
        entry_points=[CommandHandler("attack", attack_start), MessageHandler(filters.Text("⚔️ Attack"), attack_start)],
        states={
            GET_ATTACK_ARGS: [MessageHandler(filters.TEXT & ~filters.COMMAND, attack_input)],
        },
        fallbacks=[CommandHandler("cancel", cancel_conversation)],
    )

    set_duration_handler = ConversationHandler(
        entry_points=[CommandHandler("setduration", set_duration_start), MessageHandler(filters.Text("⏱ Set Duration"), set_duration_start)],
        states={
            GET_SET_DURATION: [MessageHandler(filters.TEXT & ~filters.COMMAND, set_duration_input)],
        },
        fallbacks=[CommandHandler("cancel", cancel_conversation)],
    )

    set_threads_handler = ConversationHandler(
        entry_points=[CommandHandler("set_threads", set_threads_start), MessageHandler(filters.Text("🧵 Set Threads"), set_threads_start)],
        states={
            GET_SET_THREADS: [MessageHandler(filters.TEXT & ~filters.COMMAND, set_threads_input)],
        },
        fallbacks=[CommandHandler("cancel", cancel_conversation)],
    )

    delete_key_handler = ConversationHandler(
        entry_points=[CommandHandler("deletekey", delete_key_start), MessageHandler(filters.Text("❌ Delete Key"), delete_key_start)],
        states={
            GET_DELETE_KEY: [MessageHandler(filters.TEXT & ~filters.COMMAND, delete_key_input)],
        },
        fallbacks=[CommandHandler("cancel", cancel_conversation)],
    )

    # Add all handlers
    application.add_handler(generate_key_handler)
    application.add_handler(redeem_key_handler)
    application.add_handler(attack_handler)
    application.add_handler(set_duration_handler)
    application.add_handler(set_threads_handler)
    application.add_handler(delete_key_handler)
    application.add_handler(CommandHandler("start", start))
    application.add_handler(MessageHandler(filters.PHOTO, handle_photo))
    application.add_handler(MessageHandler(filters.TEXT & ~filters.COMMAND, handle_button_click))

    application.run_polling()

if __name__ == '__main__':
    main()