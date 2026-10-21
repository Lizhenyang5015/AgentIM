const catalogs = {
  en: {
    welcomeTitle: "Hello, what would you like to do today?",
    messageListLabel: "Conversation messages",
    composerLabel: "Message",
    sendLabel: "Send message",
    sendAction: "Send",
    userName: "You",
  },
  zh: {
    welcomeTitle: "你好，今天想做什么？",
    messageListLabel: "对话消息",
    composerLabel: "消息内容",
    sendLabel: "发送消息",
    sendAction: "发送",
    userName: "你",
  },
};

const language = navigator.language.toLowerCase().startsWith("zh") ? "zh" : "en";
const messages = catalogs[language];

document.documentElement.lang = language === "zh" ? "zh-CN" : "en";

document.querySelectorAll("[data-i18n]").forEach((element) => {
  element.textContent = messages[element.dataset.i18n];
});

document.querySelectorAll("[data-i18n-aria-label]").forEach((element) => {
  element.setAttribute("aria-label", messages[element.dataset.i18nAriaLabel]);
});

const composer = document.querySelector("#composer");
const input = document.querySelector("#prompt-input");
const messageList = document.querySelector("#messages");
const messageTemplate = document.querySelector("#message-template");
const emptyState = document.querySelector("#empty-state");
const sendButton = composer.querySelector(".composer__send");

function resizeInput() {
  input.style.height = "auto";
  input.style.height = `${Math.min(input.scrollHeight, 150)}px`;
}

function updateComposerState() {
  sendButton.disabled = input.value.trim().length === 0;
}

function appendUserMessage(content) {
  const fragment = messageTemplate.content.cloneNode(true);
  const avatar = fragment.querySelector(".message__avatar");
  const author = fragment.querySelector(".message__author");
  const body = fragment.querySelector(".message__body");

  avatar.textContent = "ME";
  author.textContent = messages.userName;
  body.textContent = content;
  messageList.append(fragment);
  emptyState.hidden = true;
  messageList.scrollTop = messageList.scrollHeight;
}

function submitPrompt(prompt) {
  const normalizedPrompt = prompt.trim();
  if (!normalizedPrompt) {
    return;
  }

  appendUserMessage(normalizedPrompt);
  input.value = "";
  resizeInput();
  updateComposerState();
  input.focus();
}

input.addEventListener("input", () => {
  resizeInput();
  updateComposerState();
});

input.addEventListener("keydown", (event) => {
  if (event.key === "Enter" && !event.shiftKey) {
    event.preventDefault();
    composer.requestSubmit();
  }
});

composer.addEventListener("submit", (event) => {
  event.preventDefault();
  submitPrompt(input.value);
});

resizeInput();
updateComposerState();
